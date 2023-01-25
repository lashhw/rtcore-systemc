#include <systemc>
using namespace sc_core;

#include "../src/rtcore/arbiter.hpp"

template<int start>
SC_MODULE(master) {
    blocking_out<int> req;
    blocking_in<int> resp;

    SC_CTOR(master) {
        SC_THREAD(thread);
    }

    void thread() {
        int curr = start;
        while (true){
            wait(rand()%5, SC_NS);
            std::cout << name() << " @ " << sc_time_stamp() << ": request sent!" << std::endl;
            req->write(curr);
            curr++;
            int val;
            resp->read(val);
            wait(1, SC_NS);
        }
    }
};

SC_MODULE(slave) {
    blocking_in<int> req;
    blocking_out<int> resp;

    SC_CTOR(slave) {
        SC_THREAD(thread);
    }

    void thread() {
        while (true) {
            int val;
            std::cout << name() << " @ " << sc_time_stamp() << ": request sent!" << std::endl;
            req->read(val);
            wait(5, SC_NS);
            resp->write(val);
        }
    }
};

int sc_main(int, char **) {
    master<100> master_1("master_1");
    master<200> master_2("master_2");
    master<300> master_3("master_3");
    arbiter<int, int, 3> arbiter_i("arbiter");
    slave slave_i("slave");

    blocking<int> master_1_to_arbiter("master_1_to_arbiter");
    blocking<int> arbiter_to_master_1("arbiter_to_master_1");
    blocking<int> master_2_to_arbiter("master_2_to_arbiter");
    blocking<int> arbiter_to_master_2("arbiter_to_master_2");
    blocking<int> master_3_to_arbiter("master_3_to_arbiter");
    blocking<int> arbiter_to_master_3("arbiter_to_master_3");
    blocking<int> arbiter_to_slave("arbiter_to_slave");
    blocking<int> slave_to_arbiter("slave_to_arbieter");

    master_1.req(master_1_to_arbiter);
    master_1.resp(arbiter_to_master_1);
    master_2.req(master_2_to_arbiter);
    master_2.resp(arbiter_to_master_2);
    master_3.req(master_3_to_arbiter);
    master_3.resp(arbiter_to_master_3);
    arbiter_i.slave_from[0](master_1_to_arbiter);
    arbiter_i.slave_from[1](master_2_to_arbiter);
    arbiter_i.slave_from[2](master_3_to_arbiter);
    arbiter_i.slave_to[0](arbiter_to_master_1);
    arbiter_i.slave_to[1](arbiter_to_master_2);
    arbiter_i.slave_to[2](arbiter_to_master_3);
    arbiter_i.master_to(arbiter_to_slave);
    arbiter_i.master_from(slave_to_arbiter);
    slave_i.req(arbiter_to_slave);
    slave_i.resp(slave_to_arbiter);

    sc_start(100, SC_NS);
    return 0;
}