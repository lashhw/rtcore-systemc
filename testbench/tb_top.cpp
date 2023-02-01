#include <systemc>
using namespace sc_core;

#include "../src/memory.hpp"
#include "../src/shader.hpp"
#include "../src/rtcore/rtcore.hpp"

int sc_main(int, char **) {
    memory memory_i("memory", "kitchen.ply");
    shader shader_i("shader");
    rtcore rtcore_i("rtcore");

    sc_start();

    return 0;
}
