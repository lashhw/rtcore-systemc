#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

SC_MODULE(trv_ctrl) {
    struct thread_2_req_t {
        ray_and_id_t ray_and_id;
        dram_req_t dram_req;
    };

    blocking_out<dram_req_t> p_dram_req;
    blocking_in<dram_resp_t> p_dram_resp;
    blocking_in<ray_t> p_shader_ray;
    blocking_out<int> p_shader_id;
    blocking_out<result_t> p_shader_result;
    blocking_out<bbox_ctrl_req_t> p_bbox_ctrl;
    blocking_in<trv_ctrl_req_t> p_lp;
    blocking_in<trv_ctrl_req_t> p_hp;
    blocking_out<ist_ctrl_req_t> p_ist_ctrl_out;
    blocking_in<trv_ctrl_req_t> p_ist_ctrl_in;

    arbiter<trv_ctrl_req_t, void, 4> m_arbiter;

    blocking<trv_ctrl_req_t> b_arbiter_to_thread_3;
    blocking<thread_2_req_t> b_thread_3_to_thread_2;

    sync_fifo<trv_ctrl_req_t, num_working_rays> f_shader_fifo;
    sync_fifo<int, num_working_rays> f_free_fifo;

    std::stack<uint64_t> h_stk[num_working_rays];
    struct {
        bool intersected;
        float u;
        float v;
    } h_result[num_working_rays];

    SC_HAS_PROCESS(trv_ctrl);
    trv_ctrl(const sc_module_name &mn) : sc_module(mn),
                                         m_arbiter("m_arbiter"),
                                         b_arbiter_to_thread_3("b_arbiter_to_thread_3"),
                                         b_thread_3_to_thread_2("b_thread_3_to_thread_2"),
                                         f_shader_fifo("f_shader_fifo"),
                                         f_free_fifo("f_free_fifo") {
        m_arbiter.p_slave_req[0](f_shader_fifo);
        m_arbiter.p_slave_req[1](p_lp);
        m_arbiter.p_slave_req[2](p_hp);
        m_arbiter.p_slave_req[3](p_ist_ctrl_in);
        m_arbiter.p_master_req(b_arbiter_to_thread_3);
        for (int i = 0; i < num_working_rays; i++)
            f_free_fifo.direct_write(i);

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
        SC_THREAD(thread_3);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            int id = f_free_fifo.read();
            ray_t ray = p_shader_ray->read();

            wait(cycle);
            h_result[id].intersected = false;

            wait(cycle);
            trv_ctrl_req_t trv_ctrl_req = {
                .type = trv_ctrl_req_t::SHADER,
                .shader = {
                    .ray_and_id = {
                        .ray = ray,
                        .id = id
                    }
                }
            };
            f_shader_fifo.write(trv_ctrl_req);
            p_shader_id->write(id);
        }
    }

    void thread_2() {
        while (true) {
            wait(cycle);
            thread_2_req_t req = b_thread_3_to_thread_2.read();
            sc_assert(req.dram_req.type == dram_req_t::NODE);
            p_dram_req->write(req.dram_req);

            wait(cycle);
            dram_resp_t resp = p_dram_resp->read();
            if (resp.node.num_trigs == 0) {
                bbox_ctrl_req_t bbox_ctrl_req = {
                    .ray_and_id = req.ray_and_id,
                    .lp = {resp.node.lp[0], resp.node.lp[1]},
                    .left_bbox_ptr = resp.node.ptr
                };
                p_bbox_ctrl->write(bbox_ctrl_req);
            } else {
                ist_ctrl_req_t ist_ctrl_req = {
                    .ray_and_id = req.ray_and_id,
                    .num_trigs = resp.node.num_trigs,
                    .trig_ptr = resp.node.ptr,
                    .intersected = false
                };
                p_ist_ctrl_out->write(ist_ctrl_req);
            }
        }
    }

    void thread_3() {
        auto stk_op = [&](const ray_and_id_t &ray_and_id) {
            wait(cycle);  // read h_stk top & pop
            if (h_stk[ray_and_id.id].empty()) {

                wait(cycle);  // read h_result
                result_t result = {
                    .id = ray_and_id.id,
                    .intersected = h_result[ray_and_id.id].intersected,
                    .t = ray_and_id.ray.t_max,
                    .u = h_result[ray_and_id.id].u,
                    .v = h_result[ray_and_id.id].v
                };
                f_free_fifo.write(ray_and_id.id);
                p_shader_result->write(result);
            } else {
                uint64_t stk_top = h_stk[ray_and_id.id].top();
                h_stk[ray_and_id.id].pop();
                thread_2_req_t thread_2_req = {
                    .ray_and_id = ray_and_id,
                    .dram_req = {
                        .type = dram_req_t::NODE,
                        .addr = stk_top
                    }
                };
                b_thread_3_to_thread_2.write(thread_2_req);
            }
        };

        while (true) {
            wait(cycle);
            trv_ctrl_req_t trv_ctrl_req = b_arbiter_to_thread_3.read();
            switch(trv_ctrl_req.type) {
                case trv_ctrl_req_t::SHADER: {
                    thread_2_req_t thread_2_req = {
                        .ray_and_id = trv_ctrl_req.shader.ray_and_id,
                        .dram_req = {
                            .type = dram_req_t::NODE,
                            .addr = 24
                        }
                    };
                    b_thread_3_to_thread_2.write(thread_2_req);
                    break;
                }
                case trv_ctrl_req_t::BBOX: {
                    auto &bbox_result = trv_ctrl_req.bbox;
                    uint64_t left_node_ptr = bbox_result.left_node_ptr;
                    uint64_t right_node_ptr = left_node_ptr + 8;
                    if (bbox_result.left_hit) {
                        if (bbox_result.right_hit) {
                            // hit two bbox
                            if (bbox_result.left_first) {
                                // hit left bbox first
                                h_stk[bbox_result.ray_and_id.id].push(right_node_ptr);

                                wait(cycle);
                                thread_2_req_t thread_2_req = {
                                    .ray_and_id = bbox_result.ray_and_id,
                                    .dram_req = {
                                        .type = dram_req_t::NODE,
                                        .addr = left_node_ptr
                                    }
                                };
                                b_thread_3_to_thread_2.write(thread_2_req);
                            } else {
                                // hit right bbox first
                                h_stk[bbox_result.ray_and_id.id].push(left_node_ptr);

                                wait(cycle);
                                thread_2_req_t thread_2_req = {
                                    .ray_and_id = bbox_result.ray_and_id,
                                    .dram_req = {
                                        .type = dram_req_t::NODE,
                                        .addr = right_node_ptr
                                    }
                                };
                                b_thread_3_to_thread_2.write(thread_2_req);
                            }
                        } else {
                            // only hit left bbox
                            thread_2_req_t thread_2_req = {
                                .ray_and_id = bbox_result.ray_and_id,
                                .dram_req = {
                                    .type = dram_req_t::NODE,
                                    .addr = left_node_ptr
                                }
                            };
                            b_thread_3_to_thread_2.write(thread_2_req);
                        }
                    } else if (bbox_result.right_hit) {
                        // only hit right bbox
                        thread_2_req_t thread_2_req = {
                            .ray_and_id = bbox_result.ray_and_id,
                            .dram_req = {
                                 .type = dram_req_t::NODE,
                                 .addr = right_node_ptr
                            }
                        };
                        b_thread_3_to_thread_2.write(thread_2_req);
                    } else {
                        // don't hit bbox
                        stk_op(bbox_result.ray_and_id);
                    }
                    break;
                }
                case trv_ctrl_req_t::IST: {
                    if (trv_ctrl_req.ist.intersected) {
                        h_result[trv_ctrl_req.ist.ray_and_id.id].intersected = true;
                        h_result[trv_ctrl_req.ist.ray_and_id.id].u = trv_ctrl_req.ist.u;
                        h_result[trv_ctrl_req.ist.ray_and_id.id].v = trv_ctrl_req.ist.v;

                        wait(cycle);
                    }
                    stk_op(trv_ctrl_req.ist.ray_and_id);
                    break;
                }
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
