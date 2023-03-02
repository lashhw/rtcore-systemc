#include <systemc>
using namespace sc_core;

#include <memory_system.h>
#include "../src/dram.hpp"

int sc_main(int, char**) {
    dram m_dram("m_dram");
    sc_start(100000, SC_PS);
    return 0;
}