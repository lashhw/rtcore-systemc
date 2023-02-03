#ifndef RTCORE_SYSTEMC_PARAMS_HPP
#define RTCORE_SYSTEMC_PARAMS_HPP

constexpr int num_working_rays = 32;
constexpr int fifo_size = 32;
constexpr int num_lp = 4;
constexpr int num_hp = 4;

constexpr int mem_latency = 5;
constexpr int hp_latency = 5;

const sc_time half_cycle = sc_time(0.5, SC_NS);
const sc_time cycle = sc_time(1, SC_NS);

#endif //RTCORE_SYSTEMC_PARAMS_HPP
