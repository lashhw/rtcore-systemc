#include <systemc>
using namespace sc_core;

#include "../src/rtcore/sync_fifo.hpp"

constexpr int FIFO_SIZE = 10;
constexpr int NUM_CONSUMER = 1;
constexpr int NUM_PRODUCER = 3;

SC_MODULE(consumer) {
    sync_fifo_in<int, NUM_CONSUMER> fifo_in;
    SC_CTOR(consumer) {
        SC_THREAD(thread);
    }
    void thread() {
        while (true) {
            wait(rand()%2, SC_NS);
            std::cout << name() << " @ " << sc_time_stamp() << ": read request sent!" << std::endl;
            int val[NUM_CONSUMER];
            fifo_in->read(val);
            std::cout << name() << " @ " << sc_time_stamp() << ": ";
            for (int i : val)
                std::cout << i << " ";
            std::cout << "read!" << std::endl;
            wait(1, SC_NS);
        }
    }
};

SC_MODULE(producer) {
    sync_fifo_out<int, NUM_PRODUCER> fifo_out;
    SC_CTOR(producer) {
        SC_THREAD(thread);
    }
    void thread() {
        int curr = 0;
        while (true) {
            wait(rand()%2, SC_NS);
            int val[NUM_PRODUCER];
            for (int i = 0; i < NUM_PRODUCER; i++)
                val[i] = curr++;
            std::cout << name() << " @ " << sc_time_stamp() << ": write request (";
            for (int i : val)
                std::cout << i << " ";
            std::cout << ") sent!" << std::endl;
            fifo_out->write(val);
            std::cout << name() << " @ " << sc_time_stamp() << ": write request (";
            for (int i : val)
                std::cout << i << " ";
            std::cout << ") granted!" << std::endl;
            wait(1, SC_NS);
        }
    }
};

int sc_main(int, char **) {
    sync_fifo<int, FIFO_SIZE, NUM_CONSUMER, NUM_PRODUCER> sync_fifo_i("sync_fifo");
    consumer consumer_i("consumer");
    consumer_i.fifo_in(sync_fifo_i);
    producer producer_i("producer");
    producer_i.fifo_out(sync_fifo_i);

    sc_start(100, SC_NS);
    return 0;
}
