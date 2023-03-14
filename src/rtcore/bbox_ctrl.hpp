#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    blocking_out<dram_req_t> p_dram_req;
    blocking_in<dram_resp_t> p_dram_resp;
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
            dram_req_t dram_req = {
                .type = req.lp[0] ? dram_req_t::LP_BBOX : dram_req_t::HP_BBOX,
                .addr = req.left_bbox_ptr
            };
            p_dram_req->write(dram_req);

            wait(cycle);
            dram_resp_t dram_resp = p_dram_resp->read();
            bbox_req_t bbox_req = {
                .ray_and_id = req.ray_and_id,
            };
            bbox_req.left_bbox[0] = dram_resp.bbox.bounds[0];
            bbox_req.left_bbox[1] = dram_resp.bbox.bounds[1];
            bbox_req.left_bbox[2] = dram_resp.bbox.bounds[2];
            bbox_req.left_bbox[3] = dram_resp.bbox.bounds[3];
            bbox_req.left_bbox[4] = dram_resp.bbox.bounds[4];
            bbox_req.left_bbox[5] = dram_resp.bbox.bounds[5];

            wait(cycle);
            uint64_t right_bbox_ptr = req.left_bbox_ptr + (req.lp[0] ? 12 : 24);
            dram_req = {
                .type = req.lp[1] ? dram_req_t::LP_BBOX : dram_req_t::HP_BBOX,
                .addr = right_bbox_ptr
            };
            p_dram_req->write(dram_req);

            wait(cycle);
            dram_resp = p_dram_resp->read();
            bbox_req.right_bbox[0] = dram_resp.bbox.bounds[0];
            bbox_req.right_bbox[1] = dram_resp.bbox.bounds[1];
            bbox_req.right_bbox[2] = dram_resp.bbox.bounds[2];
            bbox_req.right_bbox[3] = dram_resp.bbox.bounds[3];
            bbox_req.right_bbox[4] = dram_resp.bbox.bounds[4];
            bbox_req.right_bbox[5] = dram_resp.bbox.bounds[5];
            bbox_req.left_node_ptr = right_bbox_ptr + (req.lp[1] ? 12 : 24);
            if (req.lp[0] && req.lp[1])
                p_lp->write(bbox_req);
            else
                p_hp->write(bbox_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
