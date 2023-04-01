#include <systemc>
using namespace sc_core;

#include "../src/blocking.hpp"

SC_MODULE(consumer) {
    blocking_in<int> b_in;

    SC_CTOR(consumer) {
        SC_THREAD(thread);
    }

    void thread() {
        while (true) {
            LOG << "sending read request";
            int val = b_in->read();
            LOG << "(" << val << ") read!";
            delay(rand() % 2 + 1);
        }
    }
};

SC_MODULE(producer) {
    blocking_out<int> b_out;

    SC_CTOR(producer) {
        SC_THREAD(thread);
    }

    void thread() {
        int curr = 0;
        while (true) {
            int val = curr++;
            LOG << "sending write request (" << val << ")";
            b_out->write(val);
            LOG << "write request (" << val << ") granted!";
            delay(rand() % 2 + 1);
        }
    }
};

int sc_main(int, char **) {
    blocking<int> b_channel("b_channel");

    consumer m_consumer("m_consumer");
    producer m_producer("m_producer");

    m_consumer.b_in(b_channel);
    m_producer.b_out(b_channel);

    sc_start(100 * cycle);
    return 0;
}
