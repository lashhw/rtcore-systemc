#ifndef RTCORE_SYSTEMC_PARAMS_HPP
#define RTCORE_SYSTEMC_PARAMS_HPP

constexpr int num_working_rays = 32;
constexpr int fifo_size = 16;
constexpr int rb_size = 16;
constexpr int num_lp = 4;
constexpr int num_hp = 4;
constexpr int num_ist = 4;

constexpr int dram_latency = 5;
constexpr int lp_latency = 5;
constexpr int hp_latency = 5;
constexpr int ist_latency = 5;

const sc_time half_cycle = sc_time(1, SC_PS);
const sc_time cycle = sc_time(2, SC_PS);

#endif //RTCORE_SYSTEMC_PARAMS_HPP
