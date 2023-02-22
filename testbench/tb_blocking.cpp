#include <systemc>
using namespace sc_core;

#include "../src/blocking.hpp"
#include "../src/utility.hpp"

SC_MODULE(consumer) {
    blocking_in<int> in;
    SC_CTOR(consumer) {
        SC_THREAD(thread);
    }
    void thread() {
        while (true) {
            wait(rand()%2 * cycle);
            std::cout << name() << " @ " << sc_time_stamp() << ": read request sent!" << std::endl;
            int val;
            in->read(val);
            std::cout << name() << " @ " << sc_time_stamp() << ": (" << val << ") read!" << std::endl;
            wait(cycle);
        }
    }
};

SC_MODULE(producer) {
    blocking_out<int> out;
    SC_CTOR(producer) {
        SC_THREAD(thread);
    }
    void thread() {
        int curr = 0;
        while (true) {
            wait(rand()%2 * cycle);
            int val = curr++;
            std::cout << name() << " @ " << sc_time_stamp() << ": write request (" << val << ") sent!" << std::endl;
            out->write(val);
            std::cout << name() << " @ " << sc_time_stamp() << ": write request (" << val << ") granted!" << std::endl;
            wait(cycle);
        }
    }
};

int sc_main(int, char **) {
    blocking<int> blocking_i("blocking");
    consumer consumer_i("consumer");
    consumer_i.in(blocking_i);
    producer producer_i("producer");
    producer_i.out(blocking_i);

    sc_start(100 * cycle);
    return 0;
}
