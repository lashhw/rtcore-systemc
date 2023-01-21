#include <systemc>
using namespace sc_core;

#include "../src/sync_fifo.hpp"

SC_MODULE(tb_sync_fifo) {
    sync_fifo_in<int> fifo_in;
    sync_fifo_out<int> fifo_out;
    SC_CTOR(tb_sync_fifo) {
        SC_THREAD(producer);
        SC_THREAD(consumer);
    }
    void producer() {
        int x = 0;
        while (true) {
            std::cout << sc_time_stamp() << ": write request (" << x << ") sent!" << std::endl;
            fifo_out->write(x);
            std::cout << sc_time_stamp() << ": " << x << " written!" << std::endl;
            x++;
        }
    }
    void consumer() {
        int x;
        while (true) {
            std::cout << sc_time_stamp() << ": read request sent!" << std::endl;
            fifo_in->read(x);
            std::cout << sc_time_stamp() << ": " << x << " read!" << std::endl;
            wait(10, SC_NS);
        }
    }
};

int sc_main(int, char**) {
    tb_sync_fifo tb_sync_fifo_i("test");
    sync_fifo<int, 10> sync_fifo_i("sync_fifo");
    tb_sync_fifo_i.fifo_in(sync_fifo_i);
    tb_sync_fifo_i.fifo_out(sync_fifo_i);

    sc_start(100, SC_NS);
    return 0;
}
