#include <systemc>
using namespace sc_core;

#include "../src/rtcore/arbiter.hpp"

template<int start>
SC_MODULE(master) {
    blocking_out<int> req;

    SC_CTOR(master) {
        SC_THREAD(thread);
    }

    void thread() {
        int curr = start;
        while (true){
            delay(rand() % 5);
            advance_to_write();
            LOG << "waiting to send request...";
            req->write(curr);
            LOG << "request sent! (" << curr << ")";
            curr++;
        }
    }
};

SC_MODULE(slave) {
    blocking_in<int> req;

    SC_CTOR(slave) {
        SC_THREAD(thread);
    }

    void thread() {
        while (true) {
            advance_to_read();
            LOG << "waiting to receive request...";
            int val = req->read();
            LOG << "request received! (" << val << ")";
        }
    }
};

int sc_main(int, char **) {
    master<100> master_1("master_1");
    master<200> master_2("master_2");
    master<300> master_3("master_3");
    arbiter<int, 3> arbiter_i("arbiter");
    slave slave_i("slave");

    blocking<int> master_1_to_arbiter("master_1_to_arbiter");
    blocking<int> master_2_to_arbiter("master_2_to_arbiter");
    blocking<int> master_3_to_arbiter("master_3_to_arbiter");
    blocking<int> arbiter_to_slave("arbiter_to_slave");

    master_1.req(master_1_to_arbiter);
    master_2.req(master_2_to_arbiter);
    master_3.req(master_3_to_arbiter);
    arbiter_i.p_slave[0](master_1_to_arbiter);
    arbiter_i.p_slave[1](master_2_to_arbiter);
    arbiter_i.p_slave[2](master_3_to_arbiter);
    arbiter_i.p_master(arbiter_to_slave);
    slave_i.req(arbiter_to_slave);

    sc_start(100 * cycle);
    return 0;
}