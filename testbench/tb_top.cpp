#include <systemc>
using namespace sc_core;

#include "../src/dram.hpp"
#include "../src/shader.hpp"
#include "../src/rtcore/rtcore.hpp"

int sc_main(int, char **) {
    // module instantiation
    dram m_dram("m_dram", "kitchen.ply");
    shader m_shader("m_shader", "ray_queries.bin");
    rtcore m_rtcore("m_rtcore");

    // link dram
    m_dram.p_rtcore_req(m_rtcore.p_dram_req);
    m_dram.p_rtcore_resp_1(m_rtcore.p_dram_resp_1);
    m_dram.p_rtcore_resp_2(m_rtcore.p_dram_resp_2);
    m_dram.p_rtcore_resp_3(m_rtcore.p_dram_resp_3);

    // link shader
    m_shader.p_rtcore_ray(m_rtcore.p_shader_ray);
    m_shader.p_rtcore_id(m_rtcore.p_shader_id);
    m_shader.p_rtcore_result(m_rtcore.p_shader_result);
    m_shader.p_dram_direct(m_dram);

    // link rtcore
    m_rtcore.p_dram_direct(m_dram);

    sc_start();

    return 0;
}
