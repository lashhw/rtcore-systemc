#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

#include <bvh/triangle.hpp>
#include "ist.hpp"

SC_MODULE(ist_ctrl) {
    blocking_out<dram_req_t> p_dram_req;
    blocking_in<dram_resp_t> p_dram_resp;
    blocking_in<ist_ctrl_req_t> p_trv_ctrl_in;
    blocking_out<trv_ctrl_req_t> p_trv_ctrl_out;
    blocking_out<ist_req_t> p_ist_out;
    blocking_in<ist_ctrl_req_t> p_ist_in;

    arbiter<ist_ctrl_req_t, void, 2> m_arbiter;

    blocking<ist_ctrl_req_t> b_arbiter_to_thread_1;

    SC_HAS_PROCESS(ist_ctrl);
    ist_ctrl(const sc_module_name &mn) : sc_module(mn),
                                         m_arbiter("m_arbiter"),
                                         b_arbiter_to_thread_1("b_arbiter_to_thread_1") {
        m_arbiter.p_slave_req[0](p_trv_ctrl_in);
        m_arbiter.p_slave_req[1](p_ist_in);
        m_arbiter.p_master_req(b_arbiter_to_thread_1);

        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            ist_ctrl_req_t ist_ctrl_req = b_arbiter_to_thread_1.read();

            wait(cycle);
            if (ist_ctrl_req.num_trigs == 0) {
                trv_ctrl_req_t trv_ctrl_req = {
                    .type = trv_ctrl_req_t::IST,
                    .ist = {
                        .ray_and_id = ist_ctrl_req.ray_and_id,
                        .intersected = ist_ctrl_req.intersected,
                        .u = ist_ctrl_req.u,
                        .v = ist_ctrl_req.v
                    }
                };
                p_trv_ctrl_out->write(trv_ctrl_req);
            } else {
                dram_req_t dram_req = {
                    .type = dram_req_t::TRIG,
                    .addr = ist_ctrl_req.trig_ptr
                };
                p_dram_req->write(dram_req);

                wait(cycle);
                dram_resp_t dram_resp = p_dram_resp->read();
                ist_req_t ist_req = {
                    .ray_and_id = ist_ctrl_req.ray_and_id,
                    .num_trigs = ist_ctrl_req.num_trigs,
                    .trig_ptr = ist_ctrl_req.trig_ptr,
                    .intersected = ist_ctrl_req.intersected,
                    .u = ist_ctrl_req.u,
                    .v = ist_ctrl_req.v,
                    .trig = dram_resp.trig
                };
                p_ist_out->write(ist_req);
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
