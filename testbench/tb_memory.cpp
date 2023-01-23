#include <systemc>
using namespace sc_core;

#include "../src/memory.hpp"

SC_MODULE(test) {
    sc_port<memory_if> memory_port;
    SC_CTOR(test) {
        SC_THREAD(thread);
    }
    void thread() {
        while (true) {
            mem_payload_t payload{};
            payload.type = mem_payload_t::BBOX;
            payload.idx = 0;
            memory_port->request(payload);
            for (int i = 0; i < 6; i++)
                std::cout << payload.response[0].bbox[i] << " ";
            std::cout << std::endl;
            wait(1, SC_NS);
        }
    }
};

int sc_main(int, char**) {
    test test_i("test");
    memory memory_i("memory", "kitchen.ply");
    test_i.memory_port(memory_i);
    sc_start(100, SC_NS);
    return 0;
}