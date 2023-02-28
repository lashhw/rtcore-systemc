#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

SC_MODULE(trv_ctrl) {
    blocking_out<mem_req_t> p_mem_req;
    blocking_in<mem_resp_t> p_mem_resp;
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

    std::stack<int> h_stk[num_working_rays];
    struct {
        bool intersected;
        float u;
        float v;
    } h_result[num_working_rays];

    SC_HAS_PROCESS(trv_ctrl);
    trv_ctrl(const char *mn) : sc_module(mn),
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
            trv_ctrl_req_t trv_ctrl_req;
            trv_ctrl_req.type = trv_ctrl_req_t::SHADER;
            trv_ctrl_req.shader.ray = ray;
            trv_ctrl_req.shader.id = id;
            f_shader_fifo.write(trv_ctrl_req);
            p_shader_id->write(id);
        }
    }

    void thread_2() {
        while (true) {
            wait(cycle);
            thread_2_req_t req = b_thread_3_to_thread_2.read();
            sc_assert(req.mem_req.type == mem_req_t::NODE);

            wait(cycle);
            p_mem_req->write(req.mem_req);

            wait(cycle);
            mem_resp_t resp = p_mem_resp->read();
            int num_trigs = resp.node[0];
            if (num_trigs == 0) {
                bbox_ctrl_req_t bbox_ctrl_req;
                bbox_ctrl_req.ray_and_id = req.ray_and_id;
                bbox_ctrl_req.left_node_idx = resp.node[1];
                p_bbox_ctrl->write(bbox_ctrl_req);
            } else {
                ist_ctrl_req_t ist_ctrl_req;
                ist_ctrl_req.ray_and_id = req.ray_and_id;
                ist_ctrl_req.num_trigs = num_trigs;
                ist_ctrl_req.first_trig_idx = resp.node[1];
                ist_ctrl_req.intersected = false;
                p_ist_ctrl_out->write(ist_ctrl_req);
            }
        }
    }

    void thread_3() {
        while (true) {
            wait(cycle);
            trv_ctrl_req_t trv_ctrl_req = b_arbiter_to_thread_3.read();
            switch(trv_ctrl_req.type) {
                case trv_ctrl_req_t::SHADER: {
                    thread_2_req_t thread_2_req;
                    thread_2_req.ray_and_id = trv_ctrl_req.shader;
                    thread_2_req.mem_req.type = mem_req_t::NODE;
                    thread_2_req.mem_req.idx = 0;
                    b_thread_3_to_thread_2.write(thread_2_req);
                    break;
                }
                case trv_ctrl_req_t::BBOX: {
                    bbox_result_t &bbox_result = trv_ctrl_req.bbox;
                    int left_node_idx = bbox_result.left_node_idx;
                    int right_node_idx = left_node_idx + 1;
                    if (bbox_result.left_hit) {
                        if (bbox_result.right_hit) {
                            // hit two bbox
                            if (bbox_result.left_first) {
                                // hit left bbox first
                                h_stk[bbox_result.ray_and_id.id].push(right_node_idx);

                                wait(cycle);
                                thread_2_req_t thread_2_req;
                                thread_2_req.ray_and_id = bbox_result.ray_and_id;
                                thread_2_req.mem_req.type = mem_req_t::NODE;
                                thread_2_req.mem_req.idx = left_node_idx;
                                b_thread_3_to_thread_2.write(thread_2_req);
                            } else {
                                // hit right bbox first
                                h_stk[bbox_result.ray_and_id.id].push(left_node_idx);

                                wait(cycle);
                                thread_2_req_t thread_2_req;
                                thread_2_req.ray_and_id = bbox_result.ray_and_id;
                                thread_2_req.mem_req.type = mem_req_t::NODE;
                                thread_2_req.mem_req.idx = right_node_idx;
                                b_thread_3_to_thread_2.write(thread_2_req);
                            }
                        } else {
                            // only hit left bbox
                            thread_2_req_t thread_2_req;
                            thread_2_req.ray_and_id = bbox_result.ray_and_id;
                            thread_2_req.mem_req.type = mem_req_t::NODE;
                            thread_2_req.mem_req.idx = left_node_idx;
                            b_thread_3_to_thread_2.write(thread_2_req);
                        }
                    } else if (bbox_result.right_hit) {
                        // only hit right bbox
                        thread_2_req_t thread_2_req;
                        thread_2_req.ray_and_id = bbox_result.ray_and_id;
                        thread_2_req.mem_req.type = mem_req_t::NODE;
                        thread_2_req.mem_req.idx = right_node_idx;
                        b_thread_3_to_thread_2.write(thread_2_req);
                    } else {
                        // don't hit bbox
                        stk_op(bbox_result.ray_and_id);
                    }
                    break;
                }
                case trv_ctrl_req_t::IST: {
                    if (trv_ctrl_req.ist.intersected) {
                        wait(cycle);
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

    void stk_op(const ray_and_id_t &ray_and_id) {
        wait(cycle);  // read h_stk top & pop
        if (h_stk[ray_and_id.id].empty()) {

            wait(cycle);  // read h_result
            result_t result;
            result.id = ray_and_id.id;
            result.intersected = h_result[ray_and_id.id].intersected;
            result.t = ray_and_id.ray.t_max;
            result.u = h_result[ray_and_id.id].u;
            result.v = h_result[ray_and_id.id].v;
            f_free_fifo.write(ray_and_id.id);
            p_shader_result->write(result);
        } else {
            int stk_top = h_stk[ray_and_id.id].top();
            h_stk[ray_and_id.id].pop();
            thread_2_req_t thread_2_req;
            thread_2_req.ray_and_id = ray_and_id;
            thread_2_req.mem_req.type = mem_req_t::NODE;
            thread_2_req.mem_req.idx = stk_top;
            b_thread_3_to_thread_2.write(thread_2_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
