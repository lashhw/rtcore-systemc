#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

SC_MODULE(trv_ctrl) {
    blocking_in<ray_t> p_shader_ray;
    blocking_out<int> p_shader_id;
    blocking_out<result_t> p_shader_result;
    blocking_out<bbox_ctrl_req_t> p_bbox_ctrl;
    blocking_in<trv_ctrl_req_t> p_lp;
    blocking_in<trv_ctrl_req_t> p_hp;
    blocking_out<ist_ctrl_req_t> p_ist_ctrl_out;
    blocking_in<trv_ctrl_req_t> p_ist_ctrl_in;
    sc_port<dram_direct_if> p_dram_direct;

    arbiter<trv_ctrl_req_t, 4> m_arbiter;

    blocking<trv_ctrl_req_t> b_arbiter_to_thread_2;

    sync_fifo<trv_ctrl_req_t, fifo_size> f_shader_fifo;
    sync_fifo<int, num_working_rays> f_free_fifo;

    SC_CTOR(trv_ctrl) : m_arbiter("m_arbiter"),
                        b_arbiter_to_thread_2("b_arbiter_to_thread_2"),
                        f_shader_fifo("f_shader_fifo"),
                        f_free_fifo("f_free_fifo") {
        m_arbiter.p_slave[0](f_shader_fifo);
        m_arbiter.p_slave[1](p_lp);
        m_arbiter.p_slave[2](p_hp);
        m_arbiter.p_slave[3](p_ist_ctrl_in);
        m_arbiter.p_master(b_arbiter_to_thread_2);
        for (int i = 0; i < num_working_rays; i++)
            f_free_fifo.direct_write(i);

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        while (true) {
            int id = f_free_fifo.read();
            ray_t ray = p_shader_ray->read();
            delay(1);
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
        std::stack<node_t> stk[num_working_rays];
        struct {
            bool intersected;
            float u;
            float v;
        } result[num_working_rays];

        for (auto &r : result)
            r.intersected = false;

        auto send_req = [&](const ray_and_id_t &ray_and_id, const node_t &node) {
            if (node.num_trigs == 0) {
                bbox_ctrl_req_t bbox_ctrl_req = {
                    .ray_and_id = ray_and_id,
                    .left_lp = node.left_lp,
                    .right_lp = node.right_lp,
                    .left_bbox_addr = node.addr
                };
                p_bbox_ctrl->write(bbox_ctrl_req);
            } else {
                ist_ctrl_req_t ist_ctrl_req = {
                    .ray_and_id = ray_and_id,
                    .num_trigs = node.num_trigs,
                    .trig_addr = node.addr,
                    .intersected = false
                };
                p_ist_ctrl_out->write(ist_ctrl_req);
            }
        };

        auto stk_op = [&](const ray_and_id_t &ray_and_id) {
            if (stk[ray_and_id.id].empty()) {
                result_t shader_result = {
                    .id = ray_and_id.id,
                    .intersected = result[ray_and_id.id].intersected,
                    .t = ray_and_id.ray.t_max,
                    .u = result[ray_and_id.id].u,
                    .v = result[ray_and_id.id].v
                };
                result[ray_and_id.id].intersected = false;
                f_free_fifo.write(ray_and_id.id);
                p_shader_result->write(shader_result);
            } else {
                node_t stk_top = stk[ray_and_id.id].top();
                stk[ray_and_id.id].pop();
                send_req(ray_and_id, stk_top);
            }
        };

        while (true) {
            trv_ctrl_req_t trv_ctrl_req = b_arbiter_to_thread_2.read();
            switch(trv_ctrl_req.type) {
                case trv_ctrl_req_t::SHADER: {
                    delay(1);
                    send_req(trv_ctrl_req.shader.ray_and_id, p_dram_direct->direct_get_data(dram_type_t::NODE, 24).node);
                    break;
                }
                case trv_ctrl_req_t::BBOX: {
                    auto &bbox_result = trv_ctrl_req.bbox;
                    if (bbox_result.left_hit) {
                        if (bbox_result.right_hit) {
                            // hit two bbox
                            if (bbox_result.left_first) {
                                // hit left bbox first
                                delay(1);
                                stk[bbox_result.ray_and_id.id].push(bbox_result.right_node);
                                send_req(bbox_result.ray_and_id, bbox_result.left_node);
                            } else {
                                // hit right bbox first
                                delay(1);
                                stk[bbox_result.ray_and_id.id].push(bbox_result.left_node);
                                send_req(bbox_result.ray_and_id, bbox_result.right_node);
                            }
                        } else {
                            // only hit left bbox
                            delay(1);
                            send_req(bbox_result.ray_and_id, bbox_result.left_node);
                        }
                    } else if (bbox_result.right_hit) {
                        // only hit right bbox
                        delay(1);
                        send_req(bbox_result.ray_and_id, bbox_result.right_node);
                    } else {
                        // don't hit bbox
                        delay(1);
                        stk_op(bbox_result.ray_and_id);
                    }
                    break;
                }
                case trv_ctrl_req_t::IST: {
                    delay(1);
                    if (trv_ctrl_req.ist.intersected) {
                        result[trv_ctrl_req.ist.ray_and_id.id].intersected = true;
                        result[trv_ctrl_req.ist.ray_and_id.id].u = trv_ctrl_req.ist.u;
                        result[trv_ctrl_req.ist.ray_and_id.id].v = trv_ctrl_req.ist.v;
                    }
                    delay(1);
                    stk_op(trv_ctrl_req.ist.ray_and_id);
                    break;
                }
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
