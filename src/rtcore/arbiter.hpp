#ifndef RTCORE_SYSTEMC_ARBITER_HPP
#define RTCORE_SYSTEMC_ARBITER_HPP

#include "../channel/blocking.hpp"
#include "../misc/utility.hpp"

template<typename T, int num_slaves>
SC_MODULE(arbiter) {
    blocking_in<T> p_slave[num_slaves];
    blocking_out<T> p_master;

    SC_CTOR(arbiter) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        int first_priority = 0;
        while (true) {
            advance_to_read();
            int chosen = -1;
            for (int i = 0, candidate = first_priority; i < num_slaves; i++, candidate = (candidate + 1) % num_slaves) {
                if (p_slave[candidate]->nb_readable()) {
                    chosen = candidate;
                    break;
                }
            }
            if (chosen != -1) {
                T req = p_slave[chosen]->read();
                delay(1);
                p_master->write(req);
                first_priority = (first_priority + 1) % num_slaves;
            } else {
                delay(1);
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_ARBITER_HPP
