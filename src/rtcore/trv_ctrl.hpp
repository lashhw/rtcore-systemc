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
    blocking_out<ist_ctrl_req_t> p_ist_ctrl_req;
    blocking_in<trv_ctrl_req_t> p_ist_ctrl_resp;

    arbiter<trv_ctrl_req_t, void, 4> m_arbiter;

    blocking<trv_ctrl_req_t> b_to_thread_3;
    blocking<thread_2_req_t> b_thread_3_to_thread_2;

    sync_fifo<trv_ctrl_req_t, num_working_rays> f_shader_fifo;
    sync_fifo<int, num_working_rays> f_free_fifo;

    std::stack<int> stk[num_working_rays];

    SC_HAS_PROCESS(trv_ctrl);
    trv_ctrl(sc_module_name mn) : sc_module(mn),
                                  m_arbiter("m_arbiter"),
                                  b_to_thread_3("b_to_thread_3"),
                                  b_thread_3_to_thread_2("b_thread_3_to_thread_2"),
                                  f_shader_fifo("f_shader_fifo"),
                                  f_free_fifo("f_free_fifo") {
        m_arbiter.p_slave_req[0](f_shader_fifo);
        m_arbiter.p_slave_req[1](p_lp);
        m_arbiter.p_slave_req[2](p_hp);
        m_arbiter.p_slave_req[3](p_ist_ctrl_resp);
        m_arbiter.p_master_req(b_to_thread_3);
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
            ray_and_id_t ray_and_id;
            ray_and_id.ray = ray;
            ray_and_id.id = id;
            trv_ctrl_req_t to_trv_ctrl;
            to_trv_ctrl.type = trv_ctrl_req_t::SHADER;
            to_trv_ctrl.ray_and_id = ray_and_id;
            f_shader_fifo.write(to_trv_ctrl);
            p_shader_id->write(id);

            wait(cycle);
            result[id].intersected = false;
        }
    }

    void thread_2() {
        while (true) {
            wait(cycle);
            thread_2_req_t req = b_thread_3_to_thread_2.read();

            wait(cycle);
            p_mem_req->write(req.mem_req);

            wait(cycle);
            mem_resp_t resp = p_mem_resp->read();
            int num_trigs = resp.node[0];
            if (num_trigs == 0) {
                bbox_ctrl_req_t to_bbox_ctrl;
                to_bbox_ctrl.ray_and_id = req.ray_and_id;
                to_bbox_ctrl.left_node_idx = resp.node[1];
                p_bbox_ctrl->write(to_bbox_ctrl);
            } else {
                ist_ctrl_req_t to_ist_ctrl;
                to_ist_ctrl.ray_and_id = req.ray_and_id;
                to_ist_ctrl.num_trigs = num_trigs;
                to_ist_ctrl.first_trig_idx = resp.node[1];
                p_ist_ctrl_req->write(to_ist_ctrl);
            }
        }
    }

    void thread_3() {
        while (true) {
            wait(cycle);
            trv_ctrl_req_t req = b_to_thread_3.read();
            thread_2_req_t thread_2_req;
            switch(req.type) {
                case trv_ctrl_req_t::SHADER: {
                    thread_2_req.ray_and_id = req.ray_and_id;
                    thread_2_req.mem_req.type = mem_req_t::NODE;
                    thread_2_req.mem_req.idx = 0;
                    b_thread_3_to_thread_2.write(thread_2_req);
                    break;
                }
                case trv_ctrl_req_t::BBOX: {
                    bbox_result_t &bbox_result = req.bbox_result;
                    int left_node_idx = bbox_result.left_node_idx;
                    int right_node_idx = left_node_idx + 1;
                    if (bbox_result.left_hit) {
                        if (bbox_result.right_hit) {
                            // hit two bbox
                            if (bbox_result.left_first) {
                                // hit left bbox first
                                stk[bbox_result.ray_and_id.id].push(right_node_idx);

                                wait(cycle);
                                thread_2_req.ray_and_id = bbox_result.ray_and_id;
                                thread_2_req.mem_req.type = mem_req_t::NODE;
                                thread_2_req.mem_req.idx = left_node_idx;
                                b_thread_3_to_thread_2.write(thread_2_req);
                            } else {
                                // hit right bbox first
                                stk[bbox_result.ray_and_id.id].push(left_node_idx);

                                wait(cycle);
                                thread_2_req.ray_and_id = bbox_result.ray_and_id;
                                thread_2_req.mem_req.type = mem_req_t::NODE;
                                thread_2_req.mem_req.idx = right_node_idx;
                                b_thread_3_to_thread_2.write(thread_2_req);
                            }
                        } else {
                            // only hit left bbox
                            thread_2_req.ray_and_id = bbox_result.ray_and_id;
                            thread_2_req.mem_req.type = mem_req_t::NODE;
                            thread_2_req.mem_req.idx = left_node_idx;
                            b_thread_3_to_thread_2.write(thread_2_req);
                        }
                    } else if (bbox_result.right_hit) {
                        // only hit right bbox
                        thread_2_req.ray_and_id = bbox_result.ray_and_id;
                        thread_2_req.mem_req.type = mem_req_t::NODE;
                        thread_2_req.mem_req.idx = right_node_idx;
                        b_thread_3_to_thread_2.write(thread_2_req);
                    } else {
                        // don't hit bbox
                        from_stk(bbox_result.ray_and_id);
                    }
                    break;
                }
                case trv_ctrl_req_t::IST: {
                    if (result[req.ist_result.ray_and_id.id].intersected) {
                        wait(cycle);
                        result[req.ist_result.ray_and_id.id].intersected = true;
                        result[req.ist_result.ray_and_id.id].u = req.ist_result.u;
                        result[req.ist_result.ray_and_id.id].v = req.ist_result.v;
                    }

                    wait(cycle);
                    from_stk(req.ist_result.ray_and_id);
                    break;
                }
            }
        }
    }

    void from_stk(const ray_and_id_t &ray_and_id) {
        wait(cycle);
        if (stk[ray_and_id.id].empty()) {
            // TODO: impl this
            std::cout << name() << " @ " << sc_time_stamp() << ": finished." << std::endl;
        } else {
            int stk_top = stk[ray_and_id.id].top();
            stk[ray_and_id.id].pop();
            thread_2_req_t thread_2_req;
            thread_2_req.ray_and_id = ray_and_id;
            thread_2_req.mem_req.type = mem_req_t::NODE;
            thread_2_req.mem_req.idx = stk_top;
            b_thread_3_to_thread_2.write(thread_2_req);
        }
    }

    struct {
        bool intersected;
        float u;
        float v;
    } result[num_working_rays];
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
