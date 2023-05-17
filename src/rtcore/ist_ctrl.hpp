#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

SC_MODULE(ist_ctrl) {
    blocking_out<ist_l1c_req_t> p_l1c_req;
    sync_fifo_in<ist_req_t> p_l1c_resp;
    blocking_in<ist_ctrl_req_t> p_trv_ctrl_in;
    blocking_out<trv_ctrl_req_t> p_trv_ctrl_out;
    sync_fifo_out<ist_req_t> p_ist_out;
    blocking_in<ist_ctrl_req_t> p_ist_in;
    sc_port<dram_direct_if> p_dram_direct;

    arbiter<ist_ctrl_req_t, 2> m_arbiter;

    blocking<ist_ctrl_req_t> b_arbiter_to_thread_1;
    blocking<ist_ctrl_req_t> b_thread_1_to_thread_2;

    SC_CTOR(ist_ctrl) : m_arbiter("m_arbiter"),
                        b_arbiter_to_thread_1("b_arbiter_to_thread_1"),
                        b_thread_1_to_thread_2("b_thread_1_to_thread_2") {
        m_arbiter.p_slave[0](p_trv_ctrl_in);
        m_arbiter.p_slave[1](p_ist_in);
        m_arbiter.p_master(b_arbiter_to_thread_1);

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
        SC_THREAD(thread_3);
    }

    void thread_1() {
        while (true) {
            ist_ctrl_req_t ist_ctrl_req = b_arbiter_to_thread_1.read();
            delay(1);
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
                b_thread_1_to_thread_2.write(ist_ctrl_req);
            }
        }
    }

    void thread_2() {
        while (true) {
            ist_ctrl_req_t req = b_thread_1_to_thread_2.read();
            delay(1);
            ist_l1c_req_t ist_l1c_req = {
                .addr = req.trig_addr,
                .num_bytes = 36,
                .additional = {
                    .ray_and_id = req.ray_and_id,
                    .num_trigs = req.num_trigs,
                    .trig_addr = req.trig_addr,
                    .intersected = req.intersected,
                    .u = req.u,
                    .v = req.v,
                    .trig = p_dram_direct->direct_get_data(dram_type_t::TRIG, req.trig_addr).trig
                }
            };
            p_l1c_req->write(ist_l1c_req);
        }
    }

    void thread_3() {
        while (true) {
            auto additional = p_l1c_resp->read();
            delay(1);
            p_ist_out->write(additional);
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
