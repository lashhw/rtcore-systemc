#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

#include "l1c.hpp"

SC_MODULE(bbox_ctrl) {
    blocking_out<bbox_l1c_req_t> p_l1c_req;
    nonblocking_in<bbox_l1c_resp_t> p_l1c_resp;
    blocking_in<bbox_ctrl_req_t> p_trv_ctrl;
    sync_fifo_out<bbox_req_t> p_lp;
    sync_fifo_out<bbox_req_t> p_hp;
    sc_port<dram_direct_if> p_dram_direct;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        while (true) {
            bbox_ctrl_req_t bbox_ctrl_req = p_trv_ctrl->read();
            delay(1);
            int num_bytes = (bbox_ctrl_req.left_lp ? 12 : 24) + (bbox_ctrl_req.right_lp ? 12 : 24) + 8 + 8;
            bbox_l1c_req_t bbox_l1c_req {
                .addr = bbox_ctrl_req.left_bbox_addr,
                .num_bytes = num_bytes,
                .additional = {
                    .ray_and_id = bbox_ctrl_req.ray_and_id,
                    .left_lp = bbox_ctrl_req.left_lp,
                    .right_lp = bbox_ctrl_req.right_lp
                }
            };
            p_l1c_req->write(bbox_l1c_req);
        }
    }

    void thread_2() {
        while (true) {
            advance_to_read();
            if (p_l1c_resp->readable()) {
                auto [addr, additional] = p_l1c_resp->read();
                delay(1);
                bbox_req_t bbox_req = {
                    .ray_and_id = additional.ray_and_id
                };
                bbox_req.left_bbox = p_dram_direct->direct_get_data(dram_type_t::BBOX, addr).bbox;
                addr += additional.left_lp ? 12 : 24;
                bbox_req.right_bbox = p_dram_direct->direct_get_data(dram_type_t::BBOX, addr).bbox;
                addr += additional.right_lp ? 12 : 24;
                bbox_req.left_node = p_dram_direct->direct_get_data(dram_type_t::NODE, addr).node;
                addr += 8;
                bbox_req.right_node = p_dram_direct->direct_get_data(dram_type_t::NODE, addr).node;
                if (additional.left_lp && additional.right_lp)
                    p_lp->write(bbox_req);
                else
                    p_hp->write(bbox_req);
            } else {
                delay(1);
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
