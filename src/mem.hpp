#ifndef RTCORE_SYSTEMC_MEM_HPP
#define RTCORE_SYSTEMC_MEM_HPP

#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include "third_party/happly/happly.h"
#include "params.hpp"
#include "payload_t.hpp"
#include "blocking.hpp"
#include "utility.hpp"

SC_MODULE(mem) {
    blocking_in<mem_req_t> p_rtcore_req;
    blocking_out<mem_resp_t> p_rtcore_resp;

    SC_HAS_PROCESS(mem);
    mem(sc_module_name mn, const char *model_ply_path) : sc_module(mn) {
        happly::PLYData ply_data(model_ply_path);
        std::vector<std::array<double, 3>> v_pos = ply_data.getVertexPositions();
        std::vector<std::vector<size_t>> f_idx = ply_data.getFaceIndices<size_t>();

        for (auto &face : f_idx) {
            trig_t trig;
            for (int i = 0; i < 3; i++) {
                trig.p0[i] = v_pos[face[0]][i];
                trig.p1[i] = v_pos[face[1]][i];
                trig.p2[i] = v_pos[face[2]][i];
            }
            trigs.push_back(trig);
            bvh_triangles.emplace_back(bvh::Vector3<float>(v_pos[face[0]][0], v_pos[face[0]][1], v_pos[face[0]][2]),
                                       bvh::Vector3<float>(v_pos[face[1]][0], v_pos[face[1]][1], v_pos[face[1]][2]),
                                       bvh::Vector3<float>(v_pos[face[2]][0], v_pos[face[2]][1], v_pos[face[2]][2]));
        }

        auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(bvh_triangles.data(), bvh_triangles.size());
        auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), bvh_triangles.size());
        std::cout << "global bounding box: ("
                  << global_bbox.min[0] << ", " << global_bbox.min[1] << ", " << global_bbox.min[2] << "), ("
                  << global_bbox.max[0] << ", " << global_bbox.max[1] << ", " << global_bbox.max[2] << ")" << std::endl;

        bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
        builder.build(global_bbox, bboxes.get(), centers.get(), bvh_triangles.size());

        traverser = std::make_shared<bvh::SingleRayTraverser<bvh::Bvh<float>>>(bvh);
        primitive_intersector = std::make_shared<bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>>>(bvh, bvh_triangles.data());

        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            mem_req_t req = p_rtcore_req->read();

            wait(mem_latency*cycle);
            mem_resp_t resp;
            switch (req.type) {
                case mem_req_t::BBOX:
                    for (int i = 0; i < 6; i++)
                        resp.bbox[i] = bvh.nodes[req.idx].bounds[i];
                    break;
                case mem_req_t::NODE:
                    resp.node[0] = bvh.nodes[req.idx].primitive_count;
                    resp.node[1] = bvh.nodes[req.idx].first_child_or_primitive;
                    break;
                case mem_req_t::TRIG_IDX:
                    resp.trig_idx = bvh.primitive_indices[req.idx];
                    break;
                case mem_req_t::TRIG:
                    for (int i = 0; i < 3; i++) {
                        resp.trig.p0[i] = trigs[req.idx].p0[i];
                        resp.trig.p1[i] = trigs[req.idx].p1[i];
                        resp.trig.p2[i] = trigs[req.idx].p2[i];
                    }
                    break;
            }
            p_rtcore_resp->write(resp);
        }
    }

    void direct_traverse(const ray_t &ray, bool &intersected, float &t, float &u, float &v) {
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

    bvh::Bvh<float> bvh;
    std::vector<bvh::Triangle<float>> bvh_triangles;
    std::vector<trig_t> trigs;
    std::shared_ptr<bvh::SingleRayTraverser<bvh::Bvh<float>>> traverser;
    std::shared_ptr<bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>>> primitive_intersector;
};

#endif //RTCORE_SYSTEMC_MEM_HPP
