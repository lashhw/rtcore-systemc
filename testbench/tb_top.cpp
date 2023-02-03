#include <systemc>
using namespace sc_core;

#include "../src/mem.hpp"
#include "../src/shader.hpp"
#include "../src/rtcore/rtcore.hpp"

int sc_main(int, char **) {
    // module instantiation
    mem m_mem("m_mem", "kitchen.ply");
    shader m_shader("m_shader", "ray_queries.bin");
    rtcore m_rtcore("m_rtcore");

    // link memory
    m_mem.p_rtcore_req(m_rtcore.p_mem_req);
    m_mem.p_rtcore_resp(m_rtcore.p_mem_resp);

    // link shader
    m_shader.p_rtcore_ray(m_rtcore.p_shader_ray);
    m_shader.p_rtcore_id(m_rtcore.p_shader_id);
    m_shader.p_rtcore_result(m_rtcore.p_shader_result);

    sc_start();

    return 0;
}
