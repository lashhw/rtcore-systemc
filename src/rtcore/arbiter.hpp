#ifndef RTCORE_SYSTEMC_ARBITER_HPP
#define RTCORE_SYSTEMC_ARBITER_HPP

#include "../blocking.hpp"
#include "../misc.hpp"

// arbiter with response
template<typename TReq, typename TResp, int num_slaves>
struct arbiter : public sc_module {
    blocking_in<TReq> p_from_slave[num_slaves];
    blocking_out<TReq> p_to_master;
    blocking_in<TResp> p_from_master;
    blocking_out<TResp> p_to_slave[num_slaves];

    SC_CTOR(arbiter) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        sc_event_or_list data_written_event_list;
        for (int i = 0; i < num_slaves; i++)
            data_written_event_list |= p_from_slave[i]->data_written_event();
        int first = 0;

        while (true) {
            bool has_data_written = false;
            for (int i = 0; i < num_slaves; i++) {
                if (p_from_slave[i]->data_written()) {
                    has_data_written = true;
                    break;
                }
            }
            if (!has_data_written)
                wait(data_written_event_list);

            wait(half_cycle);
            int chosen = first;
            for (; !p_from_slave[chosen]->data_written(); chosen = (chosen + 1) % num_slaves);
            wait(half_cycle);

            TReq req = p_from_slave[chosen]->read();
            p_to_master->write(req);
            wait(cycle);

            TResp resp = p_from_master->read();
            p_to_slave[chosen]->write(resp);
            first = (first + 1) % num_slaves;
            wait(cycle);
        }
    }
};

// arbiter without response
template<typename TReq, int num_slaves>
struct arbiter<TReq, void, num_slaves> : public sc_module {
    blocking_in<TReq> p_from_slave[num_slaves];
    blocking_out<TReq> p_to_master;

    SC_CTOR(arbiter) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        sc_event_or_list data_written_event_list;
        for (int i = 0; i < num_slaves; i++)
            data_written_event_list |= p_from_slave[i]->data_written_event();
        int first = 0;

        while (true) {
            bool has_data_written = false;
            for (int i = 0; i < num_slaves; i++) {
                if (p_from_slave[i]->data_written()) {
                    has_data_written = true;
                    break;
                }
            }
            if (!has_data_written)
                wait(data_written_event_list);

            wait(half_cycle);
            int chosen = first;
            for (; !p_from_slave[chosen]->data_written(); chosen = (chosen + 1) % num_slaves);
            wait(half_cycle);

            TReq req = p_from_slave[chosen]->read();
            p_to_master->write(req);
            first = (first + 1) % num_slaves;
            wait(cycle);
        }
    }
};

#endif //RTCORE_SYSTEMC_ARBITER_HPP
