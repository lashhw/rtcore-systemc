#ifndef RTCORE_SYSTEMC_ARBITER_HPP
#define RTCORE_SYSTEMC_ARBITER_HPP

#include "../blocking.hpp"
#include "../params.hpp"

template<typename TReq, int num_slaves>
struct arbiter : public sc_module {
    blocking_in<TReq> p_slave_req[num_slaves];
    blocking_out<TReq> p_master_req;

    SC_CTOR(arbiter) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        sc_event_or_list data_written_event_list;
        for (int i = 0; i < num_slaves; i++)
            data_written_event_list |= p_slave_req[i]->data_written_event();
        int first = 0;

        while (true) {
            wait(cycle);
            bool has_data_written = false;
            for (int i = 0; i < num_slaves; i++) {
                if (p_slave_req[i]->data_written()) {
                    has_data_written = true;
                    break;
                }
            }
            if (!has_data_written)
                wait(data_written_event_list);

            wait(half_cycle);
            int chosen = first;
            for (; !p_slave_req[chosen]->data_written(); chosen = (chosen + 1) % num_slaves);
            wait(half_cycle);
            TReq req = p_slave_req[chosen]->read();
            p_master_req->write(req);
            first = (first + 1) % num_slaves;
        }
    }
};

#endif //RTCORE_SYSTEMC_ARBITER_HPP
