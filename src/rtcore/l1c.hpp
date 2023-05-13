#ifndef RTCORE_SYSTEMC_L1C_HPP
#define RTCORE_SYSTEMC_L1C_HPP

template <typename additional_t>
SC_MODULE(l1c) {
    sync_fifo_out<dram_req_t> p_dram_req;
    nonblocking_in<uint64_t> p_dram_resp;
    blocking_in<l1c_req_t<additional_t>> p_req;
    nonblocking_out<l1c_resp_t<additional_t>> p_resp;

    SC_CTOR(l1c) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        struct rb_entry_t {
            enum { EMPTY, PENDING, WAITING, READY } status;
            uint64_t addr;
            int num_bytes;
            additional_t additional;
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
            if (p_req->readable() && !free_fifo.empty()) {
                auto [addr, num_bytes, additional] = p_req->read();
                delay(1);
                rb_entry[free_fifo.front()] = {
                    .status = rb_entry_t::PENDING,
                    .addr = addr,
                    .num_bytes = num_bytes,
                    .additional = additional,
                };
                free_fifo.pop();
                continue;
            }
            // process PENDING
            delay(1);
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
            if (ready_idx != -1) {
                l1c_resp_t<additional_t> resp {
                    .addr = rb_entry[ready_idx].addr,
                    .additional = rb_entry[ready_idx].additional
                };
                p_resp->write(resp);
                rb_entry[ready_idx].status = rb_entry_t::EMPTY;
                free_fifo.push(ready_idx);
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_L1C_HPP
