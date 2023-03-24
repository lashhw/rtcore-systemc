#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    sync_fifo_out<uint64_t> p_dram_req;
    blocking_in<uint64_t> p_dram_resp;
    blocking_in<bbox_ctrl_req_t> p_trv_ctrl;
    sync_fifo_out<bbox_req_t> p_lp;
    sync_fifo_out<bbox_req_t> p_hp;
    sc_port<dram_direct_if> p_dram_direct;

    struct {
        bool valid;
        bool pending;
        bool ready;
        uint64_t addr;
        int num_remaining;
        bool lp;
        bbox_req_t bbox_req;
    } rb[rb_size];
    std::queue<int> free_fifo;

    SC_CTOR(bbox_ctrl) {
        for (int i = 0; i < rb_size; i++)
            free_fifo.push(i);

        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(half_cycle);
            if (p_dram_resp->data_written()) {
                uint64_t addr = p_dram_resp->read();
                wait(half_cycle);
                bool found = false;
                for (auto &entry : rb) {
                    if (entry.valid && addr == entry.addr) {
                        found = true;
                        if (entry.num_remaining == 0) {
                            entry.ready = true;
                        } else {
                            entry.pending = true;
                            entry.addr += 1;
                            entry.num_remaining--;
                        }
                        break;
                    }
                }
                sc_assert(found);
            } else if (p_trv_ctrl->data_written() && !free_fifo.empty()) {
                bbox_ctrl_req_t req = p_trv_ctrl->read();
                wait(half_cycle);
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
                uint64_t first = req.left_bbox_addr >> 3;
                uint64_t last = (addr - 1) >> 3;
                rb[free_fifo.front()] = {
                    .valid = true,
                    .pending = true,
                    .ready = false,
                    .addr = first,
                    .num_remaining = int(last - first),
                    .lp = req.left_lp && req.right_lp,
                    .bbox_req = bbox_req
                };
                free_fifo.pop();
            } else {
                wait(half_cycle);
                // check pending bit
                bool has_pending = false;
                for (auto &entry : rb) {
                    if (entry.valid && entry.pending) {
                        has_pending = true;
                        if (p_dram_req->nb_writable()) {
                            p_dram_req->write(entry.addr);
                            entry.pending = false;
                        }
                        break;
                    }
                }
                if (has_pending)
                    continue;
                // check ready bit
                for (int i = 0; i < rb_size; i++) {
                    if (rb[i].valid && rb[i].ready) {
                        if (rb[i].lp && p_lp->nb_writable()) {
                            p_lp->write(rb[i].bbox_req);
                            rb[i].valid = false;
                            free_fifo.push(i);
                        } else if (!rb[i].lp && p_hp->nb_writable()) {
                            p_hp->write(rb[i].bbox_req);
                            rb[i].valid = false;
                            free_fifo.push(i);
                        }
                        break;
                    }
                }
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
