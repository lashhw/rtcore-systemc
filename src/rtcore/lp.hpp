#ifndef RTCORE_SYSTEMC_LP_HPP
#define RTCORE_SYSTEMC_LP_HPP

#include <bvh/node_intersectors.hpp>

SC_MODULE(lp) {
    sync_fifo_in<bbox_req_t, num_lp> p_bbox_ctrl;
    sync_fifo_out<trv_ctrl_req_t, num_lp> p_trv_ctrl;
    int num_processed = 0;

    SC_CTOR(lp) {
        SC_THREAD(thread_1);
    }

    ~lp() override {
        std::cout << "lp processed " << num_processed << " nodes" << std::endl;
    }

    void thread_1() {
        while (true) {
            bbox_req_t req[num_lp];
            p_bbox_ctrl->read(req);
            delay(lp_latency);
            trv_ctrl_req_t trv_ctrl_req[num_lp];
            for (int i = 0; i < num_lp; i++) {
                trv_ctrl_req[i] = {
                    .type = trv_ctrl_req_t::BBOX,
                    .bbox = {
                        .ray_and_id = req[i].ray_and_id,
                        .left_node = req[i].left_node,
                        .right_node = req[i].right_node
                    }
                };
                bvh::Ray<float> ray = to_bvh_ray(req[i].ray_and_id.ray);
                bvh::MPNodeIntersector<bvh::Bvh<float>, mantissa_width, exponent_width> node_intersector(ray);
                bvh::Bvh<float>::Node left_node = {};
                bvh::Bvh<float>::Node right_node = {};
                for (int j = 0; j < 6; j++) {
                    left_node.bounds[j] = req[i].left_bbox.bounds[j];
                    right_node.bounds[j] = req[i].right_bbox.bounds[j];
                }
                std::pair<float, float> t_left = node_intersector.intersect(left_node, ray, true);
                std::pair<float, float> t_right = node_intersector.intersect(right_node, ray, true);
                trv_ctrl_req[i].bbox.left_hit = t_left.first <= t_left.second;
                trv_ctrl_req[i].bbox.right_hit = t_right.first <= t_right.second;
                trv_ctrl_req[i].bbox.left_first = t_left.first <= t_right.first;
            }
            num_processed += num_lp;
            p_trv_ctrl->write(trv_ctrl_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_LP_HPP
