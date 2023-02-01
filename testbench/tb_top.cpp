#include <systemc>
using namespace sc_core;

#include "../src/memory.hpp"
#include "../src/shader.hpp"
#include "../src/rtcore/rtcore.hpp"

int sc_main(int, char **) {
    // module instantiation
    memory m_memory("m_memory", "kitchen.ply");
    shader m_shader("m_shader", "ray_queries.bin");
    rtcore m_rtcore("m_rtcore");

    // link memory
    m_memory.p_req(m_rtcore.p_mem_req);
    m_memory.p_resp(m_rtcore.p_mem_resp);

    // link shader
    m_shader.p_ray(m_rtcore.p_ray);
    m_shader.p_id(m_rtcore.p_id);
    m_shader.p_result(m_rtcore.p_result);

    sc_start(100, SC_NS);

    return 0;
}
