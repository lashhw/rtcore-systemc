#ifndef RTCORE_SYSTEMC_ARBITER_HPP
#define RTCORE_SYSTEMC_ARBITER_HPP

#include "../blocking.hpp"
#include "../misc.hpp"

// arbiter with response
template<typename TReq, typename TResp, int num_slaves>
struct arbiter : public sc_module {
    blocking_in<TReq> slave_from[num_slaves];
    blocking_out<TReq> master_to;
    blocking_in<TResp> master_from;
    blocking_out<TResp> slave_to[num_slaves];

    SC_CTOR(arbiter) {
        SC_THREAD(thread);
    }

    void thread() {
        sc_event_or_list data_written_event_list;
        for (int i = 0; i < num_slaves; i++)
            data_written_event_list |= slave_from[i]->data_written_event();
        int first = 0;
        while (true) {
            wait(data_written_event_list);
            wait(half_cycle);
            int choose = first;
            for (; !slave_from[choose]->data_written(); choose = (choose + 1) % num_slaves);
            wait(half_cycle);
            TReq req;
            slave_from[choose]->read(req);
            master_to->write(req);
            TResp resp;
            master_from->read(resp);
            slave_to[choose]->write(resp);
            first = (first + 1) % num_slaves;
        }
    }
};

// arbiter without response
template<typename TReq, int num_slaves>
struct arbiter<TReq, void, num_slaves> : public sc_module {
    blocking_in<TReq> slave_from[num_slaves];
    blocking_out<TReq> master_to;

    SC_CTOR(arbiter) {
        SC_THREAD(thread);
    }

    void thread() {
        sc_event_or_list data_written_event_list;
        for (int i = 0; i < num_slaves; i++)
            data_written_event_list |= slave_from[i]->data_written_event();
        int first = 0;
        while (true) {
            wait(data_written_event_list);
            wait(half_cycle);
            int choose = first;
            for (; !slave_from[choose]->data_written(); choose = (choose + 1) % num_slaves);
            wait(half_cycle);
            TReq req;
            slave_from[choose]->read(req);
            master_to->write(req);
            first = (first + 1) % num_slaves;
        }
    }
};

#endif //RTCORE_SYSTEMC_ARBITER_HPP
