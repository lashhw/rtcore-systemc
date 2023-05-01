#ifndef RTCORE_SYSTEMC_PARAMS_HPP
#define RTCORE_SYSTEMC_PARAMS_HPP

constexpr int num_working_rays = 32;
constexpr int fifo_size = 16;
constexpr int rb_size = 16;
constexpr int num_lp = 4;
constexpr int num_hp = 4;
constexpr int num_ist = 4;

constexpr mpfr_prec_t mantissa_width = 7;
constexpr mpfr_exp_t exponent_width = 8;
constexpr double t_trav_high = 0.5;
constexpr double t_trav_low = 0.47;

constexpr int dram_latency_per_byte = 5;
constexpr int lp_latency = 5;
constexpr int hp_latency = 5;
constexpr int ist_latency = 5;

const sc_time cycle = sc_time(3, SC_PS);
const sc_time phase = sc_time(1, SC_PS);

#endif //RTCORE_SYSTEMC_PARAMS_HPP
