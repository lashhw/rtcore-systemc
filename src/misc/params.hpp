#ifndef RTCORE_SYSTEMC_PARAMS_HPP
#define RTCORE_SYSTEMC_PARAMS_HPP

constexpr int num_working_rays = 32;
constexpr int fifo_size = 32;
constexpr int rb_size = 32;

int l1c_lp_num_entries = 512;  // 512 * 40 = 20 KiB
int l1c_hp_num_entries = 512;  // 512 * 64 = 32 KiB
int l1c_ist_num_entries = 512;  // 512 * 36 = 18 KiB

constexpr mpfr_prec_t mantissa_width = 7;
constexpr mpfr_exp_t exponent_width = 8;
double t_trav_high = 0.5;
double t_trav_low = 0.47;

int num_lp = 4;
int num_hp = 4;
int num_ist = 1;

constexpr int dram_latency_per_byte = 5;
constexpr int lp_latency = 20;
constexpr int hp_latency = 20;
constexpr int ist_latency = 32;

const sc_time cycle = sc_time(3, SC_PS);
const sc_time phase = sc_time(1, SC_PS);

#endif //RTCORE_SYSTEMC_PARAMS_HPP
