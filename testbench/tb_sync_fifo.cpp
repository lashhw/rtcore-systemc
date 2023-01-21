#include <systemc>
using namespace sc_core;

#include "../src/sync_fifo.hpp"

constexpr int FIFO_SIZE = 10;
constexpr int NUM_CONSUMER = 1;
constexpr int NUM_PRODUCER = 3;

SC_MODULE(consumer) {
    sync_fifo_in<int> fifo_in;
    SC_CTOR(consumer) {
        SC_THREAD(thread);
    }
    void thread() {
        while (true) {
            wait(rand()%2, SC_NS);
            std::cout << name() << " @ " << sc_time_stamp() << ": read request sent!" << std::endl;
            int x;
            fifo_in->read(x);
            std::cout << name() << " @ " << sc_time_stamp() << ": " << x << " read!" << std::endl;
        }
    }
};

SC_MODULE(producer) {
    sync_fifo_out<int> fifo_out;
    SC_CTOR(producer) {
        SC_THREAD(thread);
    }
    void thread() {
        int curr = 0;
        while (true) {
            wait(rand()%2, SC_NS);
            std::cout << name() << " @ " << sc_time_stamp() << ": write request (" << curr << ") sent!" << std::endl;
            fifo_out->write(curr);
            std::cout << name() << " @ " << sc_time_stamp() << ": " << curr << " written!" << std::endl;
            curr++;
        }
    }
};

int sc_main(int, char**) {
    sync_fifo<int, FIFO_SIZE, NUM_CONSUMER, NUM_PRODUCER> sync_fifo_i("sync_fifo");
    for (int i = 0; i < NUM_CONSUMER; i++) {
        std::string name = "consumer_" + std::to_string(i);
        auto tmp = new consumer(name.c_str());
        tmp->fifo_in(sync_fifo_i);
    }
    for (int i = 0; i < NUM_PRODUCER; i++) {
        std::string name = "producer_" + std::to_string(i);
        auto tmp = new producer(name.c_str());
        tmp->fifo_out(sync_fifo_i);
    }

    sc_start(100, SC_NS);
    return 0;
}
