#include <systemc>
using namespace sc_core;

#include "../src/dram.hpp"
#include "../src/shader.hpp"
#include "../src/rtcore/rtcore.hpp"

int sc_main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "usage: ./tb_top MODEL_PLY RAY_QUERIES [LP_L1C_ENTRIES] [HP_L1C_ENTRIES] [IST_L1C_ENTRIES] [T_TRAV_HIGH] [T_TRAV_LOW] [NUM_LP] [NUM_HP] [NUM_IST]" << std::endl;
        return EXIT_FAILURE;
    }

    if (argc >= 4)
        l1c_lp_num_entries = atoi(argv[3]);
    if (argc >= 5)
        l1c_hp_num_entries = atoi(argv[4]);
    if (argc >= 6)
        l1c_ist_num_entries = atoi(argv[5]);
    if (argc >= 7)
        t_trav_high = atof(argv[6]);
    if (argc >= 8)
        t_trav_low = atof(argv[7]);
    if (argc >= 9)
        num_lp = atoi(argv[8]);
    if (argc >= 10)
        num_hp = atoi(argv[9]);
    if (argc >= 11)
        num_ist = atoi(argv[10]);

    std::cout << "l1c_lp_num_entries = " << l1c_lp_num_entries << std::endl;
    std::cout << "l1c_hp_num_entries = " << l1c_hp_num_entries << std::endl;
    std::cout << "l1c_ist_num_entries = " << l1c_ist_num_entries << std::endl;
    std::cout << "t_trav_high = " << t_trav_high << std::endl;
    std::cout << "t_trav_low = " << t_trav_low << std::endl;
    std::cout << "num_lp = " << num_lp << std::endl;
    std::cout << "num_hp = " << num_hp << std::endl;
    std::cout << "num_ist = " << num_ist << std::endl;

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
