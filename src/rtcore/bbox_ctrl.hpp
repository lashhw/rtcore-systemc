#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    blocking_out<to_memory_t> p_memory_req;
    blocking_in<from_memory_t> p_memory_resp;
    blocking_in<to_bbox_ctrl_t> p_trv_ctrl;
    blocking_out<to_bbox_t> p_lp;
    blocking_out<to_bbox_t> p_hp;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            to_bbox_ctrl_t bbox_req = p_trv_ctrl->read();
            to_memory_t mem_req;
            mem_req.type = to_memory_t::BBOX;
            mem_req.idx = bbox_req.node_idx;
            p_memory_req->write(mem_req);
            wait(cycle);

            from_memory_t mem_resp = p_memory_resp->read();
            to_bbox_t to_bbox;
            for (int i = 0; i < 6; i++)
                to_bbox.left_bbox[i] = mem_resp.bbox[i];
            p_hp->write(to_bbox);
            wait(cycle);

            mem_req.type = to_memory_t::BBOX;
            mem_req.idx = bbox_req.node_idx;
            p_memory_req->write(mem_req);
            wait(cycle);

            mem_resp = p_memory_resp->read();
            for (int i = 0; i < 6; i++)
                to_bbox.left_bbox[i] = mem_resp.bbox[i];
            p_hp->write(to_bbox);
            wait(cycle);
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
