#ifndef RTCORE_SYSTEMC_ARBITER_HPP
#define RTCORE_SYSTEMC_ARBITER_HPP

#include "../blocking.hpp"
#include "../params.hpp"

template<typename T, int num_slaves>
struct arbiter : public sc_module {
    blocking_in<T> p_slave[num_slaves];
    blocking_out<T> p_master;

    SC_CTOR(arbiter) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        sc_event_or_list data_written_event_list;
        for (int i = 0; i < num_slaves; i++)
            data_written_event_list |= p_slave[i]->data_written_event();
        int first = 0;

        while (true) {
            wait(cycle);
            bool has_data_written = false;
            for (int i = 0; i < num_slaves; i++) {
                if (p_slave[i]->nb_readable()) {
                    has_data_written = true;
                    break;
                }
            }
            if (!has_data_written)
                wait(data_written_event_list);

            wait(half_cycle);
            int chosen = first;
            for (; !p_slave[chosen]->nb_readable(); chosen = (chosen + 1) % num_slaves);
            wait(half_cycle);
            T req = p_slave[chosen]->read();
            p_master->write(req);
            first = (first + 1) % num_slaves;
        }
    }
};

#endif //RTCORE_SYSTEMC_ARBITER_HPP
