#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"
#include "arbiter.hpp"

template <int num_working_rays>
struct trv_ctrl : public sc_module {
    sync_fifo_in<shader_to_trv_ctrl_t> shader_in_p;
    sync_fifo_in<int> lp_in_p;
    sync_fifo_in<int> hp_in_p;
    sync_fifo_in<int> ist_in_p;
    sync_fifo_out<int> shader_out_p;
    sync_fifo_out<int> bbox_ctrl_out_p;
    sync_fifo_out<int> ist_ctrl_out_p;
    sc_port<memory_if> arbiter_p;

    sync_fifo<shader_to_trv_ctrl_t, num_working_rays> shader_fifo;
    sync_fifo<int, num_working_rays> free_fifo;
    std::stack<int> stk;

    sc_event c_thread_to_b_thread_valid;
    to_memory_t c_thread_to_b_thread;

    SC_CTOR(trv_ctrl) {
        for (int i = 0; i < num_working_rays; i++)
            free_fifo.direct_write(&i);
        SC_THREAD(a_thread);
        SC_THREAD(b_thread);
        SC_THREAD(c_thread);
    }

    void a_thread() {
        while (true) {
            shader_to_trv_ctrl_t payload{};
            shader_in_p->read(&payload);
            free_fifo->read(&payload.id);
            shader_fifo.write(payload);
            wait(cycle);
        }
    }

    void b_thread() {
        while (true) {
            wait(c_thread_to_b_thread_valid);
            assert(c_thread_to_b_thread.type == to_memory_t::NODE);
            arbiter_p->request(c_thread_to_b_thread);
        }
    }

    void c_thread() {
        const sc_event * in_event[3] = {
            &shader_in_p->write_updated_event(),
            &lp_in_p->write_updated_event(),
            &hp_in_p->write_updated_event()
        };
        const int * num_elements[3] = {
            &shader_in_p->num_elements(),
            &lp_in_p->num_elements(),
            &hp_in_p->num_elements()
        };
        int first = 0;
        while (true) {
            wait(*in_event[0] | *in_event[1] | *in_event[2]);
            wait(half_cycle);
            int choose = first;
            for (; *num_elements[choose] == 0; choose = (choose + 1) % 3);
            wait(half_cycle);
            switch(choose) {
                case 0: {
                    shader_to_trv_ctrl_t payload{};
                    shader_in_p->read(&payload);
                    break;
                }
                case 1: {
                    int request;
                    lp_in_p->read(&request);
                    break;
                }
                case 2: {
                    int request;
                    hp_in_p->read(request);
                    break;
                }
                default:
                    sc_assert(false);
            }
            first = (first + 1) % 3;
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
