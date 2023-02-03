#ifndef RTCORE_SYSTEMC_MEM_HPP
#define RTCORE_SYSTEMC_MEM_HPP

#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include "third_party/happly/happly.h"
#include "misc.hpp"
#include "payload_t.hpp"
#include "blocking.hpp"

SC_MODULE(mem) {
    blocking_in<to_mem_t> p_req;
    blocking_out<from_mem_t> p_resp;

    SC_HAS_PROCESS(mem);
    mem(sc_module_name mn, const char *model_ply_path) : sc_module(mn) {
        happly::PLYData ply_data(model_ply_path);
        std::vector<std::array<double, 3>> v_pos = ply_data.getVertexPositions();
        std::vector<std::vector<size_t>> f_idx = ply_data.getFaceIndices<size_t>();

        for (auto &face : f_idx) {
            triangles.emplace_back(bvh::Vector3<float>(v_pos[face[0]][0], v_pos[face[0]][1], v_pos[face[0]][2]),
                                   bvh::Vector3<float>(v_pos[face[1]][0], v_pos[face[1]][1], v_pos[face[1]][2]),
                                   bvh::Vector3<float>(v_pos[face[2]][0], v_pos[face[2]][1], v_pos[face[2]][2]));
        }

        auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
        auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), triangles.size());
        std::cout << "global bounding box: ("
                  << global_bbox.min[0] << ", " << global_bbox.min[1] << ", " << global_bbox.min[2] << "), ("
                  << global_bbox.max[0] << ", " << global_bbox.max[1] << ", " << global_bbox.max[2] << ")" << std::endl;

        bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
        builder.build(global_bbox, bboxes.get(), centers.get(), triangles.size());

        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            to_mem_t req = p_req->read();
            wait(mem_latency*cycle);

            from_mem_t resp;
            switch (req.type) {
                case to_mem_t::BBOX:
                    for (int i = 0; i < 6; i++)
                        resp.bbox[i] = bvh.nodes[req.idx].bounds[i];
                    break;
                case to_mem_t::NODE:
                    resp.node[0] = bvh.nodes[req.idx].primitive_count;
                    resp.node[1] = bvh.nodes[req.idx].first_child_or_primitive;
                    break;
                case to_mem_t::TRIG_IDX:
                    resp.trig_idx = bvh.primitive_indices[req.idx];
                    break;
                case to_mem_t::TRIG:
                    for (int i = 0; i < 3; i++) {
                        resp.trig.p0[i] = triangles[req.idx].p0[i];
                        resp.trig.e1[i] = triangles[req.idx].e1[i];
                        resp.trig.e2[i] = triangles[req.idx].e2[i];
                    }
                    break;
            }
            p_resp->write(resp);
            wait(cycle);
        }
    }

    std::vector<bvh::Triangle<float>> triangles;
    bvh::Bvh<float> bvh;
};

#endif //RTCORE_SYSTEMC_MEM_HPP
