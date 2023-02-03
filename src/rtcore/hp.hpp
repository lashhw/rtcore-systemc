#ifndef RTCORE_SYSTEMC_HP_HPP
#define RTCORE_SYSTEMC_HP_HPP

#include <bvh/node_intersectors.hpp>
#include <bvh/bvh.hpp>
#include "../params.hpp"

SC_MODULE(hp) {
    sync_fifo_in<bbox_req_t, num_hp> p_bbox_ctrl;
    sync_fifo_out<trv_ctrl_req_t, num_hp> p_trv_ctrl;

    SC_CTOR(hp) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            bbox_req_t req[num_hp];
            p_bbox_ctrl->read(req);

            wait(hp_latency*cycle);
            trv_ctrl_req_t trv_ctrl_req[num_hp];
            for (int i = 0; i < num_hp; i++) {
                trv_ctrl_req[i].type = trv_ctrl_req_t::BBOX;
                bbox_result_t &bbox_result = trv_ctrl_req[i].bbox_result;
                bbox_result.ray_and_id = req[i].ray_and_id;
                bbox_result.left_node_idx = req[i].left_node_idx;
                bvh::Ray<float> ray(
                    {req[i].ray_and_id.ray.origin[0], req[i].ray_and_id.ray.origin[1], req[i].ray_and_id.ray.origin[2]},
                    {req[i].ray_and_id.ray.dir[0], req[i].ray_and_id.ray.dir[1], req[i].ray_and_id.ray.dir[2]},
                    req[i].ray_and_id.ray.t_min,
                    req[i].ray_and_id.ray.t_max
                );
                bvh::FastNodeIntersector<bvh::Bvh<float>> node_intersector(ray);
                bvh::Bvh<float>::Node left_node, right_node;
                for (int j = 0; j < 6; j++) {
                    left_node.bounds[j] = req[i].left_bbox[j];
                    right_node.bounds[j] = req[i].right_bbox[j];
                }
                std::pair<float, float> t_left = node_intersector.intersect(left_node, ray);
                std::pair<float, float> t_right = node_intersector.intersect(right_node, ray);
                bbox_result.left_hit = t_left.first <= t_left.second;
                bbox_result.right_hit = t_right.first <= t_right.second;
                bbox_result.left_first = t_left.first <= t_right.first;
            }
            p_trv_ctrl->write(trv_ctrl_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_HP_HPP
