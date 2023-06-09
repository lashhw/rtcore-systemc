#ifndef RTCORE_SYSTEMC_LP_HPP
#define RTCORE_SYSTEMC_LP_HPP

#include <bvh/node_intersectors.hpp>

SC_MODULE(lp) {
    sync_fifo_in<bbox_req_t> p_bbox_ctrl;
    sync_fifo_out<trv_ctrl_req_t> p_trv_ctrl;
    int num_processed = 0;

    SC_CTOR(lp) {
        SC_THREAD(thread_1);
    }

    ~lp() override {
        std::cout << "lp processed " << num_processed << " nodes" << std::endl;
    }

    void thread_1() {
        std::vector<std::pair<trv_ctrl_req_t, int>> processing;

        while (true) {
            // read: read request
            advance_to_read();
            if (processing.size() < num_lp && p_bbox_ctrl->readable()) {
                bbox_req_t bbox_req = p_bbox_ctrl->read();
                trv_ctrl_req_t trv_ctrl_req = {
                    .type = trv_ctrl_req_t::BBOX
                };
                trv_ctrl_req.bbox = {
                    .ray_and_id = bbox_req.ray_and_id,
                    .left_node = bbox_req.left_node,
                    .right_node = bbox_req.right_node
                };
                bvh::Ray<float> ray = to_bvh_ray(bbox_req.ray_and_id.ray);
                bvh::MPNodeIntersector<bvh::Bvh<float>, mantissa_width, exponent_width> node_intersector(ray);
                bvh::Bvh<float>::Node left_node = {};
                bvh::Bvh<float>::Node right_node = {};
                for (int j = 0; j < 6; j++) {
                    left_node.bounds[j] = bbox_req.left_bbox.bounds[j];
                    right_node.bounds[j] = bbox_req.right_bbox.bounds[j];
                }
                std::pair<float, float> t_left = node_intersector.intersect(left_node, ray, true);
                std::pair<float, float> t_right = node_intersector.intersect(right_node, ray, true);
                trv_ctrl_req.bbox.left_hit = t_left.first <= t_left.second;
                trv_ctrl_req.bbox.right_hit = t_right.first <= t_right.second;
                trv_ctrl_req.bbox.left_first = t_left.first <= t_right.first;

                processing.emplace_back(trv_ctrl_req, lp_latency);
                num_processed++;
            }

            // update: update counter
            advance_to_update();
            for (auto &p : processing)
                p.second = std::min(p.second, p.second - 1);

            // write: send response
            delay(1);
            advance_to_write();
            for (auto it = processing.begin(); it != processing.end(); it++) {
                if (it->second <= 0) {
                    p_trv_ctrl->write(it->first);
                    processing.erase(it);
                    break;
                }
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_LP_HPP
