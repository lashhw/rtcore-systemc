#include <systemc>
using namespace sc_core;

#include "../src/dram.hpp"
#include "../src/shader.hpp"
#include "../src/rtcore/rtcore.hpp"

int sc_main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "usage: ./tb_top [model.ply] [ray_queries.bin]" << std::endl;
        return EXIT_FAILURE;
    }

    // module instantiation
    dram m_dram("m_dram", argv[1]);
    shader m_shader("m_shader", argv[2]);
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
