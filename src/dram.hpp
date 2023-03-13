#ifndef RTCORE_SYSTEMC_DRAM_HPP
#define RTCORE_SYSTEMC_DRAM_HPP

#include <unordered_map>

SC_MODULE(dram) {
    blocking_in<mem_req_t> p_rtcore_req;
    blocking_out<mem_resp_t> p_rtcore_resp;

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
            bvh_triangles.emplace_back(bvh::Vector3<float>(float(v_pos[face[0]][0]),
                                                           float(v_pos[face[0]][1]),
                                                           float(v_pos[face[0]][2])),
                                       bvh::Vector3<float>(float(v_pos[face[1]][0]),
                                                           float(v_pos[face[1]][1]),
                                                           float(v_pos[face[1]][2])),
                                       bvh::Vector3<float>(float(v_pos[face[2]][0]),
                                                           float(v_pos[face[2]][1]),
                                                           float(v_pos[face[2]][2])));
        }

        // build bvh
        auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(bvh_triangles.data(), bvh_triangles.size());
        auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), bvh_triangles.size());
        std::cout << "global bounding box: ("
                  << global_bbox.min[0] << ", " << global_bbox.min[1] << ", " << global_bbox.min[2] << "), ("
                  << global_bbox.max[0] << ", " << global_bbox.max[1] << ", " << global_bbox.max[2] << ")" << std::endl;
        bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
        builder.build(global_bbox, bboxes.get(), centers.get(), bvh_triangles.size());

        // permute triangles
        auto permuted_trigs = bvh::permute_primitives(trigs.data(), bvh.primitive_indices.get(), trigs.size());
        auto permuted_bvh_triangles = bvh::permute_primitives(bvh_triangles.data(), bvh.primitive_indices.get(), bvh_triangles.size());
        for (int i = 0; i < bvh_triangles.size(); i++) {
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

        // construct dram node data
        int curr_addr = 0;
        // std::unordered_map<int, int> node_addr_invmap;  // node_index -> dram_addr
        for (int i = 0; i < bvh.node_count; i++) {
            node_addr_invmap[i] = curr_addr;

            node_t node{};
            if (node.num_trigs == 0) {
                int left_child_idx = int(bvh.nodes[i].first_child_or_primitive);
                int right_child_idx = left_child_idx + 1;
                node.lp[0] = low_precision[left_child_idx];
                node.lp[1] = low_precision[right_child_idx];
            }
            node.num_trigs = int(bvh.nodes[i].primitive_count);
            for (int j = 0; j < 6; j++)
                node.bbox[j] = bvh.nodes[i].bounds[j];
            nodes.push_back(node);

            if (low_precision[i])
                curr_addr += 20;
            else
                curr_addr += 32;
        }

        // construct dram trig data
        // std::unordered_map<int, int> trig_addr_invmap;  // trig_index -> dram_addr
        for (int i = 0; i < bvh_triangles.size(); i++) {
            trig_addr_invmap[i] = curr_addr;
            curr_addr += 36;
        }

        // fill in ptr
        for (int i = 0; i < nodes.size(); i++) {
            int idx = int(bvh.nodes[i].first_child_or_primitive);
            nodes[i].ptr = nodes[i].num_trigs == 0 ? node_addr_invmap[idx] : trig_addr_invmap[idx];
        }

        // fill in addr_map
        for (int i = 0; i < nodes.size(); i++)
            addr_map[node_addr_invmap[i]] = &nodes[i];
        for (int i = 0; i < trigs.size(); i++)
            addr_map[trig_addr_invmap[i]] = &trigs[i];

        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            mem_req_t req = p_rtcore_req->read();

            wait(mem_latency*cycle);
            mem_resp_t resp{};
            switch (req.type) {
                case mem_req_t::BBOX: {
                    int addr = node_addr_invmap[req.idx];
                    auto node = (node_t*)addr_map[addr];
                    resp.bbox.low_precision = false;
                    for (int i = 0; i < 6; i++)
                        resp.bbox.bounds[i] = node->bbox[i];
                    break;
                }
                case mem_req_t::NODE: {
                    resp.node[0] = int(bvh.nodes[req.idx].primitive_count);
                    resp.node[1] = int(bvh.nodes[req.idx].first_child_or_primitive);
                    break;
                }
                case mem_req_t::TRIG_IDX: {
                    resp.trig_idx = req.idx;
                    break;
                }
                case mem_req_t::TRIG: {
                    int addr = trig_addr_invmap[req.idx];
                    auto trig = (trig_t*)addr_map[addr];
                    for (int i = 0; i < 3; i++) {
                        resp.trig.p0[i] = trig->p0[i];
                        resp.trig.p1[i] = trig->p1[i];
                        resp.trig.p2[i] = trig->p2[i];
                    }
                    break;
                }
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

    std::unordered_map<int, void*> addr_map;
    std::vector<node_t> nodes;
    std::vector<trig_t> trigs;

    bvh::Bvh<float> bvh;
    std::vector<bvh::Triangle<float>> bvh_triangles;
    std::shared_ptr<bvh::SingleRayTraverser<bvh::Bvh<float>>> traverser;
    std::shared_ptr<bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>, true>> primitive_intersector;

    // tmp
    std::unordered_map<int, int> node_addr_invmap;  // node_index -> dram_addr
    std::unordered_map<int, int> trig_addr_invmap;  // trig_index -> dram_addr
};

#endif //RTCORE_SYSTEMC_DRAM_HPP
