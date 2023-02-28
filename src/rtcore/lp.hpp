#ifndef RTCORE_SYSTEMC_LP_HPP
#define RTCORE_SYSTEMC_LP_HPP

SC_MODULE(lp) {
    sync_fifo_in<bbox_req_t, num_lp> p_bbox_ctrl;
    sync_fifo_out<trv_ctrl_req_t, num_lp> p_trv_ctrl;

    SC_CTOR(lp) {
            SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            bbox_req_t req[num_lp];
            p_bbox_ctrl->read(req);

            wait(lp_latency * cycle);
            trv_ctrl_req_t trv_ctrl_req[num_lp];
            for (int i = 0; i < num_lp; i++) {
                trv_ctrl_req[i] = {
                    .type = trv_ctrl_req_t::BBOX,
                    .bbox = {
                        .ray_and_id = req[i].ray_and_id,
                        .left_node_idx = req[i].left_node_idx
                    }
                };
                bvh::Ray<float> ray = to_bvh_ray(req[i].ray_and_id.ray);
                bvh::FastNodeIntersector<bvh::Bvh<float>> node_intersector(ray);
                bvh::Bvh<float>::Node left_node{}, right_node{};
                for (int j = 0; j < 6; j++) {
                    left_node.bounds[j] = req[i].left_bbox[j];
                    right_node.bounds[j] = req[i].right_bbox[j];
                }
                std::pair<float, float> t_left = node_intersector.intersect(left_node, ray);
                std::pair<float, float> t_right = node_intersector.intersect(right_node, ray);
                trv_ctrl_req[i].bbox.left_hit = t_left.first <= t_left.second;
                trv_ctrl_req[i].bbox.right_hit = t_right.first <= t_right.second;
                trv_ctrl_req[i].bbox.left_first = t_left.first <= t_right.first;
            }
            p_trv_ctrl->write(trv_ctrl_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_LP_HPP
