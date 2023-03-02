#ifndef RTCORE_SYSTEMC_DRAM_HPP
#define RTCORE_SYSTEMC_DRAM_HPP

#include "params.hpp"

SC_MODULE(dram) {
    SC_HAS_PROCESS(dram);
    dram(const sc_module_name &mn) : sc_module(mn),
                                      clk(0),
                                      memory_system("dram_config.ini", ".", std::bind(&dram::ReadCallBack, this, std::placeholders::_1), std::bind(&dram::WriteCallBack, this, std::placeholders::_1)) {
        SC_THREAD(thread_1);
    }

    void ReadCallBack(uint64_t addr) {
        std::cout << "READ 0x" << std::hex << std::uppercase << addr << " " << std::dec << clk << std::endl;
    }

    void WriteCallBack(uint64_t addr) {
        std::cout << "WRITE 0x" << std::hex << std::uppercase << addr << " " << std::dec << clk << std::endl;
    }

    void thread_1() {
        bool get_next = true;
        dramsim3::Transaction trans;
        std::ifstream trace_file("example.trace");
        while (true) {
            wait(cycle);
            clk++;
            memory_system.ClockTick();
            if (!trace_file.eof()) {
                if (get_next) {
                    get_next = false;
                    trace_file >> trans;
                }
                if (trans.added_cycle <= clk) {
                    get_next = memory_system.WillAcceptTransaction(trans.addr, trans.is_write);
                    if (get_next) {
                        memory_system.AddTransaction(trans.addr, trans.is_write);
                    }
                }
            }
        }
    }

    uint64_t clk;
    dramsim3::MemorySystem memory_system;
};

#endif //RTCORE_SYSTEMC_DRAM_HPP
