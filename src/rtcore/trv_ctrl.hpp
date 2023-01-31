#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

template <int num_working_rays>
struct trv_ctrl : public sc_module {
    sync_fifo_in<bvh::Ray<float>> shader_in_req_p;
    sync_fifo_out<int> shader_in_resp_p;
    sync_fifo_in<void*> lp_in_p;
    sync_fifo_in<void*> hp_in_p;
    sync_fifo_in<void*> ist_in_p;
    sync_fifo_out<void*> shader_out_p;
    sync_fifo_out<void*> bbox_ctrl_out_p;
    sync_fifo_out<void*> ist_ctrl_out_p;
    sync_fifo_out<to_memory_t> arbiter_out_req_p;
    sync_fifo_in<from_memory_t> arbiter_out_resp_p;

    arbiter<to_trv_ctrl_t, vo

    sync_fifo<to_trv_ctrl_t, num_working_rays> shader_fifo;
    sync_fifo<int, num_working_rays> free_fifo;
    std::stack<int> stk[num_working_rays];

    sc_event c_thread_to_b_thread_event;
    to_memory_t c_thread_to_b_thread;

    SC_CTOR(trv_ctrl) {
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
            arbiter_out_req_p->write(c_thread_to_b_thread);
            from_memory_t resp;
            arbiter_out_resp_p->read(resp);
            // other
            // remember to prevent multiple request simultaneously
        }
    }

    void c_thread() {
        const sc_event *in_event[4] = {
            &shader_fifo.data_written_event(),
            &lp_in_p->data_written_event(),
            &hp_in_p->data_written_event(),
            &ist_in_p->data_written_event()
        };
        const int *num_elements[4] = {
            &shader_fifo.num_elements(),
            &lp_in_p->num_elements(),
            &hp_in_p->num_elements(),
            &ist_in_p->num_elements()
        };
        int first = 0;
        while (true) {
            wait(*in_event[0] | *in_event[1] | *in_event[2] | *in_event[3]);
            wait(half_cycle);
            int choose = first;
            for (; *num_elements[choose] == 0; choose = (choose + 1) % 4);
            wait(half_cycle);
            switch(choose) {
                case 0: {
                    break;
                }
                case 1: {
                    break;
                }
                case 2: {
                    break;
                }
                case 3: {
                    break;
                }
                default: {
                    sc_assert(false);
                }
            }
            first = (first + 1) % 4;
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
