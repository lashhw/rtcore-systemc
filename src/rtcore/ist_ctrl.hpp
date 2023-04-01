#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

SC_MODULE(ist_ctrl) {
    sync_fifo_out<dram_req_t> p_dram_req;
    nonblocking_in<uint64_t> p_dram_resp;
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
        struct rb_entry_t {
            enum { EMPTY, PENDING, WAITING, READY } status;
            uint64_t addr;
            int num_bytes;
            ist_req_t ist_req;
        } rb_entry[rb_size];
        std::queue<int> free_fifo;
        for (int i = 0; i < rb_size; i++) {
            rb_entry[i].status = rb_entry_t::EMPTY;
            free_fifo.push(i);
        }

        while (true) {
            advance_to_read();
            // process WAITING
            if (p_dram_resp->readable()) {
                uint64_t addr = p_dram_resp->read();
                int addr_idx = -1;
                for (int i = 0; i < rb_size; i++) {
                    if (rb_entry[i].status == rb_entry_t::WAITING && addr == rb_entry[i].addr) {
                        addr_idx = i;
                        break;
                    }
                }
                if (addr_idx != -1) {
                    delay(1);
                    rb_entry[addr_idx].status = rb_entry_t::READY;
                    continue;
                }
            }
            // process EMPTY
            if (b_thread_1_to_thread_2.readable() && !free_fifo.empty()) {
                ist_ctrl_req_t req = b_thread_1_to_thread_2.read();
                delay(1);
                ist_req_t ist_req = {
                    .ray_and_id = req.ray_and_id,
                    .num_trigs = req.num_trigs,
                    .trig_addr = req.trig_addr,
                    .intersected = req.intersected,
                    .u = req.u,
                    .v = req.v,
                    .trig = p_dram_direct->direct_get_data(dram_type_t::TRIG, req.trig_addr).trig
                };
                rb_entry[free_fifo.front()] = {
                    .status = rb_entry_t::PENDING,
                    .addr = req.trig_addr,
                    .num_bytes = 36,
                    .ist_req = ist_req
                };
                free_fifo.pop();
                continue;
            }
            delay(1);
            // process PENDING
            int pending_idx = -1;
            for (int i = 0; i < rb_size; i++) {
                if (rb_entry[i].status == rb_entry_t::PENDING) {
                    pending_idx = i;
                    break;
                }
            }
            if (pending_idx != -1 && p_dram_req->writable()) {
                dram_req_t req = {
                    .addr = rb_entry[pending_idx].addr,
                    .num_bytes = rb_entry[pending_idx].num_bytes
                };
                p_dram_req->write(req);
                rb_entry[pending_idx].status = rb_entry_t::WAITING;
                continue;
            }
            // process READY
            int ready_idx = -1;
            for (int i = 0; i < rb_size; i++) {
                if (rb_entry[i].status == rb_entry_t::READY) {
                    ready_idx = i;
                    break;
                }
            }
            if (ready_idx != -1 && p_ist_out->writable()) {
                p_ist_out->write(rb_entry[ready_idx].ist_req);
                rb_entry[ready_idx].status = rb_entry_t::EMPTY;
                free_fifo.push(ready_idx);
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
