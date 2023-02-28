#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    blocking_out<mem_req_t> p_mem_req;
    blocking_in<mem_resp_t> p_mem_resp;
    blocking_in<bbox_ctrl_req_t> p_trv_ctrl;
    blocking_out<bbox_req_t> p_lp;
    blocking_out<bbox_req_t> p_hp;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            bbox_ctrl_req_t req = p_trv_ctrl->read();
            mem_req_t mem_req = {
                .type = mem_req_t::BBOX,
                .idx = req.left_node_idx
            };
            p_mem_req->write(mem_req);

            wait(cycle);
            mem_resp_t mem_resp = p_mem_resp->read();
            bbox_req_t bbox_req = {
                .ray_and_id = req.ray_and_id,
                .left_node_idx = req.left_node_idx
            };
            for (int i = 0; i < 6; i++)
                bbox_req.left_bbox[i] = mem_resp.bbox.bounds[i];

            wait(cycle);
            mem_req.type = mem_req_t::BBOX;
            mem_req.idx = req.left_node_idx + 1;
            p_mem_req->write(mem_req);

            wait(cycle);
            mem_resp = p_mem_resp->read();
            for (int i = 0; i < 6; i++)
                bbox_req.right_bbox[i] = mem_resp.bbox.bounds[i];
            if (mem_resp.bbox.low_precision)
                p_lp->write(bbox_req);
            else
                p_hp->write(bbox_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
