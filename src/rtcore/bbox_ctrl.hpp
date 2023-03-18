#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    blocking_out<uint64_t> p_dram_req;
    blocking_in<uint64_t> p_dram_resp;
    blocking_in<bbox_ctrl_req_t> p_trv_ctrl;
    blocking_out<bbox_req_t> p_lp;
    blocking_out<bbox_req_t> p_hp;
    sc_port<dram_direct_if> p_dram_direct;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            bbox_ctrl_req_t req = p_trv_ctrl->read();
            bbox_req_t bbox_req = {
                .ray_and_id = req.ray_and_id
            };
            uint64_t addr = req.left_bbox_addr;
            bbox_req.left_bbox = p_dram_direct->direct_get_data(dram_type_t::BBOX, addr).bbox;
            addr += req.left_lp ? 12 : 24;
            bbox_req.right_bbox = p_dram_direct->direct_get_data(dram_type_t::BBOX, addr).bbox;
            addr += req.right_lp ? 12 : 24;
            bbox_req.left_node = p_dram_direct->direct_get_data(dram_type_t::NODE, addr).node;
            addr += 8;
            bbox_req.right_node = p_dram_direct->direct_get_data(dram_type_t::NODE, addr).node;
            addr += 8;
            if (req.left_lp && req.right_lp)
                p_lp->write(bbox_req);
            else
                p_hp->write(bbox_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
