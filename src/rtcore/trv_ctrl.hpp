#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

SC_MODULE(trv_ctrl) {
    blocking_out<to_memory_t> p_memory_req;
    blocking_in<from_memory_t> p_memory_resp;
    blocking_in<bvh::Ray<float>> p_ray;
    blocking_out<int> p_id;
    sync_fifo_out<to_shader_t> p_result;
    sync_fifo_out<to_bbox_ctrl_t> p_bbox_ctrl;
    sync_fifo_in<to_trv_ctrl_t> p_lp;
    sync_fifo_in<to_trv_ctrl_t> p_hp;
    sync_fifo_out<to_ist_ctrl_t> p_ist_ctrl_req;
    sync_fifo_in<to_trv_ctrl_t> p_ist_ctrl_resp;

    SC_CTOR(trv_ctrl) {

    }
    /*
    arbiter<to_trv_ctrl_t, void, 4> trv_ctrl_arbiter;
    blocking<to_trv_ctrl_t> trv_ctrl_arbiter_out;
    blocking<to_b_thread_t> c_thread_to_b_thread;

    sync_fifo<to_trv_ctrl_t, num_working_rays> shader_fifo;
    sync_fifo<int, num_working_rays> free_fifo;
    std::stack<int> stk[num_working_rays];

    SC_HAS_PROCESS(trv_ctrl);
    trv_ctrl(sc_module_name mn) : sc_module(mn),
                                  trv_ctrl_arbiter("trv_ctrl_arbiter") {
        trv_ctrl_arbiter.slave_from[0](shader_fifo);
        trv_ctrl_arbiter.slave_from[1](p_lp);
        trv_ctrl_arbiter.slave_from[2](p_hp);
        trv_ctrl_arbiter.slave_from[3](p_ist);
        trv_ctrl_arbiter.master_to(trv_ctrl_arbiter_out);
        for (int i = 0; i < num_working_rays; i++)
            free_fifo.direct_write(i);
        SC_THREAD(a_thread);
        SC_THREAD(b_thread);
        SC_THREAD(c_thread);
    }

    void a_thread() {
        while (true) {
            bvh::Ray<float> ray = p_shader_req->read();
            int id = free_fifo.read();
            ray_and_id_t ray_and_id{ray, id};
            to_trv_ctrl_t to_trv_ctrl;
            to_trv_ctrl.type = to_trv_ctrl_t::SHADER;
            to_trv_ctrl.ray_and_id = ray_and_id;
            shader_fifo.write(to_trv_ctrl);
            p_shader_resp->write(id);
        }
    }

    void b_thread() {
        while (true) {
            to_b_thread_t req = c_thread_to_b_thread.read();
            p_memory_req->write(req.to_memory);
            from_memory_t resp = p_memory_resp->read();
            size_t num_trigs = resp.node[0];
            if (num_trigs == 0) {
                to_bbox_ctrl_t to_bbox_ctrl{};
                to_bbox_ctrl.ray_and_id = req.ray_and_id;
                to_bbox_ctrl.node_idx = resp.node[1];
                p_bbox_ctrl->write(to_bbox_ctrl);
            } else {
                to_ist_ctrl_t to_ist_ctrl{};
                to_ist_ctrl.ray_and_id = req.ray_and_id;
                to_ist_ctrl.num_trigs = num_trigs;
                to_ist_ctrl.first_trig_idx = resp.node[1];
                p_ist_ctrl->write(to_ist_ctrl);
            }
        }
    }

    void c_thread() {
        while (true) {
            to_trv_ctrl_t chosen = trv_ctrl_arbiter_out.read();
            switch(chosen.type) {
                case to_trv_ctrl_t::SHADER: {
                    to_b_thread_t to_b_thread;
                    to_b_thread.ray_and_id = chosen.ray_and_id;
                    to_b_thread.to_memory.type = to_memory_t::NODE;
                    to_b_thread.to_memory.idx = 0;
                    c_thread_to_b_thread.write(to_b_thread);
                    break;
                }
                case to_trv_ctrl_t::BBOX: {
                    int left_node_idx = chosen.from_bbox.left_node_idx;
                    int right_node_idx = left_node_idx + 1;
                    // TODO: add stack latency
                    if (chosen.from_bbox.hit[0]) {
                        if (chosen.from_bbox.hit[1]) {
                            // hit two bbox
                            if (chosen.from_bbox.left_first) {
                                // hit left bbox first
                                stk[chosen.ray_and_id.id].push(right_node_idx);
                                to_b_thread_t to_b_thread;
                                to_b_thread.ray_and_id = chosen.ray_and_id;
                                to_b_thread.to_memory.type = to_memory_t::NODE;
                                to_b_thread.to_memory.idx = left_node_idx;
                                c_thread_to_b_thread.write(to_b_thread);
                            } else {
                                // hit right bbox first
                                stk[chosen.ray_and_id.id].push(left_node_idx);
                                to_b_thread_t to_b_thread;
                                to_b_thread.ray_and_id = chosen.ray_and_id;
                                to_b_thread.to_memory.type = to_memory_t::NODE;
                                to_b_thread.to_memory.idx = right_node_idx;
                                c_thread_to_b_thread.write(to_b_thread);
                            }
                        } else {
                            // only hit left bbox
                            to_b_thread_t to_b_thread;
                            to_b_thread.ray_and_id = chosen.ray_and_id;
                            to_b_thread.to_memory.type = to_memory_t::NODE;
                            to_b_thread.to_memory.idx = left_node_idx;
                            c_thread_to_b_thread.write(to_b_thread);
                        }
                    } else if (chosen.from_bbox.hit[1]) {
                        // only hit right bbox
                        to_b_thread_t to_b_thread;
                        to_b_thread.ray_and_id = chosen.ray_and_id;
                        to_b_thread.to_memory.type = to_memory_t::NODE;
                        to_b_thread.to_memory.idx = right_node_idx;
                        c_thread_to_b_thread.write(to_b_thread);
                    } else {
                        // don't hit bbox
                        if (stk[chosen.ray_and_id.id].empty()) {
                            // TODO: impl this
                            to_shader_t to_shader;
                        } else {
                            to_b_thread_t to_b_thread;
                            to_b_thread.ray_and_id = chosen.ray_and_id;
                            to_b_thread.to_memory.type = to_memory_t::NODE;
                            to_b_thread.to_memory.idx = stk[chosen.ray_and_id.id].top();
                            stk[chosen.ray_and_id.id].pop();
                            c_thread_to_b_thread.write(to_b_thread);
                        }
                    }
                    to_bbox_ctrl_t to_bbox_ctrl;
                    to_bbox_ctrl.ray_and_id = chosen.ray_and_id;
                    break;
                }
                case to_trv_ctrl_t::IST: {
                    break;
                }
                default: {
                    sc_assert(false);
                }
            }
        }
    }
     */
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
