#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

SC_MODULE(trv_ctrl) {
    blocking_out<to_memory_t> p_memory_req;
    blocking_in<from_memory_t> p_memory_resp;
    blocking_in<ray_t> p_ray;
    blocking_out<int> p_id;
    blocking_out<result_t> p_result;
    blocking_out<to_bbox_ctrl_t> p_bbox_ctrl;
    blocking_in<to_trv_ctrl_t> p_lp;
    blocking_in<to_trv_ctrl_t> p_hp;
    blocking_out<to_ist_ctrl_t> p_ist_ctrl_req;
    blocking_in<to_trv_ctrl_t> p_ist_ctrl_resp;

    arbiter<to_trv_ctrl_t, void, 4> m_arbiter;

    blocking<to_trv_ctrl_t> b_to_thread_3;
    blocking<to_thread_2_t> b_thread_3_to_thread_2;

    sync_fifo<to_trv_ctrl_t, num_working_rays> f_shader_fifo;
    sync_fifo<int, num_working_rays> f_free_fifo;

    std::stack<int> stk[num_working_rays];

    SC_HAS_PROCESS(trv_ctrl);
    trv_ctrl(sc_module_name mn) : sc_module(mn),
                                  m_arbiter("m_arbiter"),
                                  b_to_thread_3("b_to_thread_3"),
                                  b_thread_3_to_thread_2("b_thread_3_to_thread_2"),
                                  f_shader_fifo("f_shader_fifo"),
                                  f_free_fifo("f_free_fifo") {
        m_arbiter.slave_from[0](f_shader_fifo);
        m_arbiter.slave_from[1](p_lp);
        m_arbiter.slave_from[2](p_hp);
        m_arbiter.slave_from[3](p_ist_ctrl_resp);
        m_arbiter.master_to(b_to_thread_3);
        for (int i = 0; i < num_working_rays; i++)
            f_free_fifo.direct_write(i);

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
        SC_THREAD(thread_3);
    }

    void thread_1() {
        while (true) {
            ray_t ray = p_ray->read();
            int id = f_free_fifo.read();
            wait(cycle);
            ray_and_id_t ray_and_id{ray, id};
            to_trv_ctrl_t to_trv_ctrl;
            to_trv_ctrl.type = to_trv_ctrl_t::SHADER;
            to_trv_ctrl.ray_and_id = ray_and_id;
            f_shader_fifo.write(to_trv_ctrl);
            p_id->write(id);
            wait(cycle);
        }
    }

    void thread_2() {
        while (true) {
            to_thread_2_t req = b_thread_3_to_thread_2.read();
            wait(cycle);
            p_memory_req->write(req.to_memory);
            wait(cycle);
            from_memory_t resp = p_memory_resp->read();
            int num_trigs = resp.node[0];
            if (num_trigs == 0) {
                to_bbox_ctrl_t to_bbox_ctrl;
                to_bbox_ctrl.ray_and_id = req.ray_and_id;
                to_bbox_ctrl.node_idx = resp.node[1];
                p_bbox_ctrl->write(to_bbox_ctrl);
            } else {
                to_ist_ctrl_t to_ist_ctrl;
                to_ist_ctrl.ray_and_id = req.ray_and_id;
                to_ist_ctrl.num_trigs = num_trigs;
                to_ist_ctrl.first_trig_idx = resp.node[1];
                p_ist_ctrl_req->write(to_ist_ctrl);
            }
            wait(cycle);
        }
    }

    void thread_3() {
        while (true) {
            to_trv_ctrl_t req = b_to_thread_3.read();
            switch(req.type) {
                case to_trv_ctrl_t::SHADER: {
                    to_thread_2_t to_thread_2;
                    to_thread_2.ray_and_id = req.ray_and_id;
                    to_thread_2.to_memory.type = to_memory_t::NODE;
                    to_thread_2.to_memory.idx = 0;
                    b_thread_3_to_thread_2.write(to_thread_2);
                    wait(cycle);
                    break;
                }
                case to_trv_ctrl_t::BBOX: {
                    int left_node_idx = req.from_bbox.left_node_idx;
                    int right_node_idx = left_node_idx + 1;
                    if (req.from_bbox.hit[0]) {
                        if (req.from_bbox.hit[1]) {
                            // hit two bbox
                            if (req.from_bbox.left_first) {
                                // hit left bbox first
                                stk[req.from_bbox.ray_and_id.id].push(right_node_idx);
                                wait(cycle);
                                to_thread_2_t to_thread_2;
                                to_thread_2.ray_and_id = req.from_bbox.ray_and_id;
                                to_thread_2.to_memory.type = to_memory_t::NODE;
                                to_thread_2.to_memory.idx = left_node_idx;
                                b_thread_3_to_thread_2.write(to_thread_2);
                                wait(cycle);
                            } else {
                                // hit right bbox first
                                stk[req.from_bbox.ray_and_id.id].push(left_node_idx);
                                wait(cycle);
                                to_thread_2_t to_thread_2;
                                to_thread_2.ray_and_id = req.from_bbox.ray_and_id;
                                to_thread_2.to_memory.type = to_memory_t::NODE;
                                to_thread_2.to_memory.idx = right_node_idx;
                                b_thread_3_to_thread_2.write(to_thread_2);
                                wait(cycle);
                            }
                        } else {
                            // only hit left bbox
                            to_thread_2_t to_thread_2;
                            to_thread_2.ray_and_id = req.from_bbox.ray_and_id;
                            to_thread_2.to_memory.type = to_memory_t::NODE;
                            to_thread_2.to_memory.idx = left_node_idx;
                            b_thread_3_to_thread_2.write(to_thread_2);
                            wait(cycle);
                        }
                    } else if (req.from_bbox.hit[1]) {
                        // only hit right bbox
                        to_thread_2_t to_thread_2;
                        to_thread_2.ray_and_id = req.from_bbox.ray_and_id;
                        to_thread_2.to_memory.type = to_memory_t::NODE;
                        to_thread_2.to_memory.idx = right_node_idx;
                        b_thread_3_to_thread_2.write(to_thread_2);
                        wait(cycle);
                    } else {
                        // don't hit bbox
                        if (stk[req.from_bbox.ray_and_id.id].empty()) {
                            // TODO: impl this
                        } else {
                            int stk_top = stk[req.from_bbox.ray_and_id.id].top();
                            stk[req.from_bbox.ray_and_id.id].pop();
                            wait(cycle);
                            to_thread_2_t to_thread_2;
                            to_thread_2.ray_and_id = req.from_bbox.ray_and_id;
                            to_thread_2.to_memory.type = to_memory_t::NODE;
                            to_thread_2.to_memory.idx = stk_top;
                            b_thread_3_to_thread_2.write(to_thread_2);
                            wait(cycle);
                        }
                    }
                    break;
                }
                case to_trv_ctrl_t::IST: {
                    break;
                }
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
