#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

template <int num_working_rays>
struct trv_ctrl : public sc_module {
    sync_fifo_in<bvh::Ray<float>> shader_in_req_p;
    sync_fifo_out<int> shader_in_resp_p;
    sync_fifo_in<to_trv_ctrl_t> lp_in_p;
    sync_fifo_in<to_trv_ctrl_t> hp_in_p;
    sync_fifo_in<to_trv_ctrl_t> ist_in_p;
    sync_fifo_out<void*> shader_out_p;
    sync_fifo_out<void*> bbox_ctrl_out_p;
    sync_fifo_out<void*> ist_ctrl_out_p;
    sync_fifo_out<to_memory_t> memory_out_req_p;
    sync_fifo_in<from_memory_t> memory_out_resp_p;

    arbiter<sync_fifo_in, blocking_out, to_trv_ctrl_t, void, 4> trv_ctrl_arbiter;
    blocking<to_trv_ctrl_t> trv_ctrl_arbiter_out;

    sync_fifo<to_trv_ctrl_t, num_working_rays> shader_fifo;
    sync_fifo<int, num_working_rays> free_fifo;
    std::stack<int> stk[num_working_rays];

    sc_event c_thread_to_b_thread_event;
    to_memory_t c_thread_to_b_thread;

    SC_HAS_PROCESS(trv_ctrl);
    trv_ctrl(sc_module_name mn) : sc_module(mn),
                                  trv_ctrl_arbiter("trv_ctrl_arbiter") {
        trv_ctrl_arbiter.slave_from[0](shader_fifo);
        trv_ctrl_arbiter.slave_from[1](lp_in_p);
        trv_ctrl_arbiter.slave_from[2](hp_in_p);
        trv_ctrl_arbiter.slave_from[3](ist_in_p);
        trv_ctrl_arbiter.master_to(trv_ctrl_arbiter_out);
        for (int i = 0; i < num_working_rays; i++)
            free_fifo.direct_write(i);
        SC_THREAD(a_thread);
        SC_THREAD(b_thread);
        SC_THREAD(c_thread);
    }

    void a_thread() {
        while (true) {
            bvh::Ray<float> ray;
            shader_in_req_p->read(ray);
            int id;
            free_fifo.read(id);
            ray_and_id_t ray_and_id{ray, id};
            to_trv_ctrl_t to_trv_ctrl;
            to_trv_ctrl.type = to_trv_ctrl_t::SHADER;
            to_trv_ctrl.payload.ray_and_id = ray_and_id;
            shader_fifo.write(to_trv_ctrl);
            shader_in_resp_p->write(id);
        }
    }

    void b_thread() {
        while (true) {
            wait(c_thread_to_b_thread_event);
            assert(c_thread_to_b_thread.type == to_memory_t::NODE);
            memory_out_req_p->write(c_thread_to_b_thread);
            from_memory_t resp;
            memory_out_resp_p->read(resp);
            // other
            // remember to prevent multiple request simultaneously
        }
    }

    void c_thread() {
        while (true) {
            to_trv_ctrl_t chosen;
            trv_ctrl_arbiter_out.read(chosen);
            switch(chosen.type) {
                case to_trv_ctrl_t::SHADER: {
                    break;
                }
                case to_trv_ctrl_t::LP: {
                    break;
                }
                case to_trv_ctrl_t::HP: {
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
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
