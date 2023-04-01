#include <systemc>
using namespace sc_core;

#include "../src/channel/nonblocking.hpp"
#include "../src/misc/utility.hpp"

SC_MODULE(consumer) {
    nonblocking_in<int> n_in;

    SC_CTOR(consumer) {
        SC_THREAD(thread);
    }

    void thread() {
        while (true) {
            advance_to_read();
            if (n_in->readable()) {
                int val = n_in->read();
                LOG << "(" << val << ") read!";
            }
            delay(1);
        }
    }
};

SC_MODULE(producer) {
    nonblocking_out<int> n_out;

    SC_CTOR(producer) {
        SC_THREAD(thread);
    }

    void thread() {
        int curr = 0;
        while (true) {
            int val = curr++;
            advance_to_write();
            n_out->write(val);
            LOG << "write request (" << val << ") sent!";
            delay(rand() % 5 + 1);
        }
    }
};

int sc_main(int, char **) {
    nonblocking<int> n_channel("n_channel");

    consumer m_consumer("m_consumer");
    producer m_producer("m_producer");

    m_consumer.n_in(n_channel);
    m_producer.n_out(n_channel);

    sc_start(100 * cycle);
    return 0;
}
