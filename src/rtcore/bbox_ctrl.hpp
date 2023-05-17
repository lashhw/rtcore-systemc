#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

#include "l1c.hpp"

SC_MODULE(bbox_ctrl) {
    blocking_out<bbox_l1c_req_t> p_l1c_lp_req;
    sync_fifo_in<bbox_req_t> p_l1c_lp_resp;
    blocking_out<bbox_l1c_req_t> p_l1c_hp_req;
    sync_fifo_in<bbox_req_t> p_l1c_hp_resp;
    blocking_in<bbox_ctrl_req_t> p_trv_ctrl;
    sync_fifo_out<bbox_req_t> p_lp;
    sync_fifo_out<bbox_req_t> p_hp;
    sc_port<dram_direct_if> p_dram_direct;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
        SC_THREAD(thread_3);
    }

    void thread_1() {
        while (true) {
            bbox_ctrl_req_t req = p_trv_ctrl->read();
            delay(1);
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
            bbox_l1c_req_t bbox_l1c_req {
                .addr = req.left_bbox_addr,
                .num_bytes = int(addr - req.left_bbox_addr),
                .additional = bbox_req
            };
            if (req.left_lp && req.right_lp)
                p_l1c_lp_req->write(bbox_l1c_req);
            else
                p_l1c_hp_req->write(bbox_l1c_req);
        }
    }

    void thread_2() {
        while (true) {
            auto additional = p_l1c_lp_resp->read();
            delay(1);
            p_lp->write(additional);
        }
    }

    void thread_3() {
        while (true) {
            auto additional = p_l1c_hp_resp->read();
            delay(1);
            p_hp->write(additional);
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
