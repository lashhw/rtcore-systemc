#ifndef RTCORE_SYSTEMC_TRV_CTRL_HPP
#define RTCORE_SYSTEMC_TRV_CTRL_HPP

#include "../payload_t.hpp"

template <int num_working_rays>
struct trv_ctrl : public sc_module {
    sync_fifo_in<int> shader_in_p;
    sync_fifo_in<int> lp_in_p;
    sync_fifo_in<int> hp_in_p;
    sync_fifo_in<int> ist_in_p;
    sync_fifo_out<int> shader_out_p;
    sync_fifo_out<int> bbox_ctrl_out_p;
    sync_fifo_out<int> ist_ctrl_out_p;
    sync_fifo_out<int> arbiter_p;

    sync_fifo<int, num_working_rays> free_fifo;

    SC_CTOR(trv_ctrl) {
        for (int i = 0; i < num_working_rays; i++)
            free_fifo.direct_write(i);
        SC_THREAD(a_thread);
        SC_THREAD(b_thread);
    }

    void a_thread() {
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
            wait(0.5, SC_NS);
            int choose = first;
            for (; *num_elements[choose] == 0; choose = (choose + 1) % 3);
            sc_assert(0 <= choose && choose < 3);
            wait(0.5, SC_NS);
            switch(choose) {
                case 0: {
                    int request;
                    shader_in_p->read(request);
                    break;
                }
                case 1: {
                    int request;
                    lp_in_p->read(request);
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

    void b_thread() {
        while (true) {

        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_CTRL_HPP
