#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    sync_fifo_out<dram_req_t> p_dram_req;
    nonblocking_in<uint64_t> p_dram_resp;
    blocking_in<bbox_ctrl_req_t> p_trv_ctrl;
    sync_fifo_out<bbox_req_t> p_lp;
    sync_fifo_out<bbox_req_t> p_hp;
    sc_port<dram_direct_if> p_dram_direct;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        struct {
            bool valid;
            bool pending;
            bool ready;
            uint64_t addr;
            int num_bytes;
            bool lp;
            bbox_req_t bbox_req;
        } rb_entry[rb_size];
        std::queue<int> free_fifo;
        for (int i = 0; i < rb_size; i++) {
            rb_entry[i].valid = false;
            free_fifo.push(i);
        }

        while (true) {
            advance_to_read();
            // process dram resp
            if (p_dram_resp->readable()) {
                uint64_t addr = p_dram_resp->read();
                int addr_idx = -1;
                for (int i = 0; i < rb_size; i++) {
                    if (rb_entry[i].valid && addr == rb_entry[i].addr) {
                        addr_idx = i;
                        break;
                    }
                }
                if (addr_idx != -1) {
                    delay(1);
                    rb_entry[addr_idx].ready = true;
                    continue;
                }
            }
            // process req from trv_ctrl
            if (p_trv_ctrl->readable() && !free_fifo.empty()) {
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
                rb_entry[free_fifo.front()] = {
                    .valid = true,
                    .pending = true,
                    .ready = false,
                    .addr = req.left_bbox_addr,
                    .num_bytes = int(addr - req.left_bbox_addr),
                    .lp = req.left_lp && req.right_lp,
                    .bbox_req = bbox_req
                };
                free_fifo.pop();
                continue;
            }
            delay(1);
            // check pending bit
            int pending_idx = -1;
            for (int i = 0; i < rb_size; i++) {
                if (rb_entry[i].valid && rb_entry[i].pending) {
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
                rb_entry[pending_idx].pending = false;
                continue;
            }
            // check ready bit
            int ready_idx = -1;
            for (int i = 0; i < rb_size; i++) {
                if (rb_entry[i].valid && rb_entry[i].ready) {
                    ready_idx = i;
                    break;
                }
            }
            if (ready_idx != -1) {
                if (rb_entry[ready_idx].lp && p_lp->writable()) {
                    p_lp->write(rb_entry[ready_idx].bbox_req);
                    rb_entry[ready_idx].valid = false;
                    free_fifo.push(ready_idx);
                } else if (!rb_entry[ready_idx].lp && p_hp->writable()) {
                    p_hp->write(rb_entry[ready_idx].bbox_req);
                    rb_entry[ready_idx].valid = false;
                    free_fifo.push(ready_idx);
                }
                continue;
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
