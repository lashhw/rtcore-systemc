#include <systemc>
using namespace sc_core;

#include "../src/channel/sync_fifo.hpp"
#include "../src/dram.hpp"

SC_MODULE(cpu) {
    blocking_out<uint64_t> p_dram_req;
    blocking_in<uint64_t> p_dram_resp;

    SC_CTOR(cpu) : p_dram_req("p_dram_req"),
                   p_dram_resp("p_dram_resp") {
        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        int i = 0;
        while (true) {
            p_dram_req->write(i);
            LOG << "request " << i << " sent";
            i++;
            delay(1);
        }
    }

    void thread_2() {
        while (true) {
            advance_to_read();
            if (p_dram_resp->nb_readable()) {
                uint64_t addr = p_dram_resp->read();
                LOG << "response " << addr << " received";
            }
            delay(1);
        }
    }
};

int sc_main(int, char **) {
    // module instantiation
    dram m_dram("m_dram", "kitchen.ply");
    cpu m_cpu("m_cpu");

    // channel instantiation
    blocking<uint64_t> b_dram_req("b_dram_req");
    blocking<uint64_t> b_dram_resp("b_dram_resp");

    // link dram
    m_dram.p_rtcore_req(b_dram_req);
    m_dram.p_rtcore_resp(b_dram_resp);

    // link req_generator
    m_cpu.p_dram_req(b_dram_req);
    m_cpu.p_dram_resp(b_dram_resp);

    sc_start(1000 * cycle);

    return 0;
}
