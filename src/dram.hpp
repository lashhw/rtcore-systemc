#ifndef RTCORE_SYSTEMC_DRAM_HPP
#define RTCORE_SYSTEMC_DRAM_HPP

#include <unordered_map>
#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include "third_party/happly/happly.h"
#include "rtcore/sync_fifo.hpp"
#include "mark.hpp"
#include "params.hpp"
#include "payload_t.hpp"
#include "blocking.hpp"
#include "utility.hpp"

struct dram_direct_if : virtual public sc_interface {
    virtual dram_data_t direct_get_data(dram_type_t type, uint64_t addr) = 0;
    virtual void direct_traverse(const ray_t &ray, bool &intersected, float &t, float &u, float &v) = 0;
};

struct dram : public sc_module,
              public dram_direct_if {
    blocking_in<uint64_t> p_rtcore_req;
    blocking_out<uint64_t> p_rtcore_resp;

    std::unordered_map<uint64_t, void*> addr_map;  // address -> pointer to actual data
    std::vector<bbox_t> bboxes;
    std::vector<node_t> nodes;
    std::vector<trig_t> trigs;

    bvh::Bvh<float> bvh;
    std::vector<bvh::Triangle<float>> bvh_triangles;
    std::shared_ptr<bvh::SingleRayTraverser<bvh::Bvh<float>>> traverser;
    std::shared_ptr<bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>, true>> primitive_intersector;

    std::vector<std::pair<uint64_t, int>> remaining_cycles;

    SC_HAS_PROCESS(dram);
    dram(const sc_module_name &mn, const char *model_ply_path) : sc_module(mn) {
        // read ply file
        happly::PLYData ply_data(model_ply_path);
        std::vector<std::array<double, 3>> v_pos = ply_data.getVertexPositions();
        std::vector<std::vector<size_t>> f_idx = ply_data.getFaceIndices<size_t>();
        for (auto &face : f_idx) {
            trig_t trig{};
            for (int i = 0; i < 3; i++) {
                trig.p0[i] = float(v_pos[face[0]][i]);
                trig.p1[i] = float(v_pos[face[1]][i]);
                trig.p2[i] = float(v_pos[face[2]][i]);
            }
            trigs.push_back(trig);
            bvh_triangles.push_back(to_bvh_triangle(trig));
        }

        // build bvh
        auto [trig_bboxes, trig_centers] = bvh::compute_bounding_boxes_and_centers(bvh_triangles.data(), bvh_triangles.size());
        auto global_bbox = bvh::compute_bounding_boxes_union(trig_bboxes.get(), bvh_triangles.size());
        std::cout << "global bounding box: ("
                  << global_bbox.min[0] << ", " << global_bbox.min[1] << ", " << global_bbox.min[2] << "), ("
                  << global_bbox.max[0] << ", " << global_bbox.max[1] << ", " << global_bbox.max[2] << ")" << std::endl;
        bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
        builder.build(global_bbox, trig_bboxes.get(), trig_centers.get(), bvh_triangles.size());

        // permute triangles
        auto permuted_trigs = bvh::permute_primitives(trigs.data(), bvh.primitive_indices.get(), trigs.size());
        auto permuted_bvh_triangles = bvh::permute_primitives(bvh_triangles.data(), bvh.primitive_indices.get(), bvh_triangles.size());
        for (int i = 0; i < trigs.size(); i++) {
            trigs[i] = permuted_trigs[i];
            bvh_triangles[i] = permuted_bvh_triangles[i];
        }

        // construct traverser and intersector
        traverser = std::make_shared<bvh::SingleRayTraverser<bvh::Bvh<float>>>(bvh);
        primitive_intersector = std::make_shared<bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>, true>>(bvh, bvh_triangles.data());

        // calculate lp/hp
        std::vector<bool> low_precision;
        HighPrecisionMarker marker(7, 8);
        marker.mark(bvh, 0.5, 0.45, low_precision);
        low_precision[0] = false;

        // construct dram bbox/node data
        int curr_addr = 0;
        std::unordered_map<int, uint64_t> bbox_addr_map;  // bbox index -> dram addr
        std::unordered_map<int, uint64_t> node_addr_map;  // node index -> dram addr
        auto construct_bbox = [&](int idx) {
            bbox_t bbox = {
                .bounds = {
                    bvh.nodes[idx].bounds[0],
                    bvh.nodes[idx].bounds[1],
                    bvh.nodes[idx].bounds[2],
                    bvh.nodes[idx].bounds[3],
                    bvh.nodes[idx].bounds[4],
                    bvh.nodes[idx].bounds[5],
                }
            };
            bboxes.push_back(bbox);
            bbox_addr_map[idx] = curr_addr;
            curr_addr += low_precision[idx] ? 12 : 24;
        };
        auto construct_node = [&](int idx) {
            node_t node = {};
            if (node.num_trigs == 0) {
                int left_child_idx = int(bvh.nodes[idx].first_child_or_primitive);
                int right_child_idx = left_child_idx + 1;
                node.left_lp = low_precision[left_child_idx];
                node.right_lp = low_precision[right_child_idx];
            }
            node.num_trigs = int(bvh.nodes[idx].primitive_count);
            nodes.push_back(node);
            node_addr_map[idx] = curr_addr;
            curr_addr += 8;
        };
        construct_bbox(0);
        construct_node(0);
        for (int i = 1; i < bvh.node_count; i += 2) {
            construct_bbox(i);
            construct_bbox(i + 1);
            construct_node(i);
            construct_node(i + 1);
        }

        // construct dram trig data
        std::unordered_map<int, uint64_t> trig_addr_map;  // trig index -> dram addr
        for (int i = 0; i < trigs.size(); i++) {
            addr_map[curr_addr] = &trigs[i];
            trig_addr_map[i] = curr_addr;
            curr_addr += 36;
        }

        // fill addr_map
        for (int i = 0; i < bboxes.size(); i++)
            addr_map[bbox_addr_map[i]] = &bboxes[i];
        for (int i = 0; i < nodes.size(); i++)
            addr_map[node_addr_map[i]] = &nodes[i];
        for (int i = 0; i < trigs.size(); i++)
            addr_map[trig_addr_map[i]] = &trigs[i];

        // fill in addr
        for (int i = 0; i < nodes.size(); i++) {
            int idx = int(bvh.nodes[i].first_child_or_primitive);
            nodes[i].addr = nodes[i].num_trigs == 0 ? bbox_addr_map[idx] : trig_addr_map[idx];
        }

        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            // negedge: read request & update counter
            ADVANCE_TO_NEGEDGE();
            if (p_rtcore_req->nb_readable()) {
                uint64_t req = p_rtcore_req->read();
                remaining_cycles.emplace_back(req, dram_latency);
            }
            for (auto &remaining_cycle : remaining_cycles)
                remaining_cycle.second--;

            // posedge: send response
            ADVANCE_TO_POSEDGE();
            for (auto it = remaining_cycles.begin(); it != remaining_cycles.end(); it++) {
                if (it->second <= 0) {
                    SHOULD_NOT_BE_BLOCKED(p_rtcore_resp->write(it->first));
                    remaining_cycles.erase(it);
                    break;
                }
            }
        }
    }

    dram_data_t direct_get_data(dram_type_t type, uint64_t addr) override {
        dram_data_t data = {};
        sc_assert(addr_map.count(addr) > 0);
        switch (type) {
            case dram_type_t::BBOX: {
                data.bbox = *(bbox_t*)addr_map[addr];
                break;
            }
            case dram_type_t::NODE: {
                data.node = *(node_t*)addr_map[addr];
                break;
            }
            case dram_type_t::TRIG: {
                data.trig = *(trig_t*)addr_map[addr];
                break;
            }
        }
        return data;
    }

    void direct_traverse(const ray_t &ray, bool &intersected, float &t, float &u, float &v) override {
        auto result = traverser->traverse(to_bvh_ray(ray), *primitive_intersector);
        if (result) {
            intersected = true;
            t = result->intersection.t;
            u = result->intersection.u;
            v = result->intersection.v;
        } else {
            intersected = false;
        }
    }
};

#endif //RTCORE_SYSTEMC_DRAM_HPP
