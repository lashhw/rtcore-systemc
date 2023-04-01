#ifndef RTCORE_SYSTEMC_UTILITY_HPP
#define RTCORE_SYSTEMC_UTILITY_HPP

#include <bvh/ray.hpp>
#include <bvh/triangle.hpp>
#include "params.hpp"
#include "payload_t.hpp"

bvh::Ray<float> to_bvh_ray(const ray_t &ray) {
    return {
        {ray.origin[0], ray.origin[1], ray.origin[2]},
        {ray.dir[0], ray.dir[1], ray.dir[2]},
        ray.t_min,
        ray.t_max
    };
}

bvh::Triangle<float> to_bvh_triangle(const trig_t &trig) {
    return {
        {trig.p0[0], trig.p0[1], trig.p0[2]},
        {trig.p1[0], trig.p1[1], trig.p1[2]},
        {trig.p2[0], trig.p2[1], trig.p2[2]}
    };
}

enum class phase_t {
    WRITE, READ, UPDATE
};

phase_t curr_phase() {
    auto phase_val = sc_time_stamp().value() % cycle.value();
    switch (phase_val) {
        case 0:
            return phase_t::WRITE;
        case 1:
            return phase_t::READ;
        case 2:
            return phase_t::UPDATE;
        default:
            SC_REPORT_FATAL("timing", "unknown phase");
            exit(EXIT_FAILURE);
    }
}

sc_time::value_type curr_cycle() {
    return sc_time_stamp().value() / cycle.value();
}

const char *curr_phase_str() {
    switch (curr_phase()) {
        case phase_t::WRITE:
            return "WRITE";
        case phase_t::READ:
            return "READ";
        case phase_t::UPDATE:
            return "UPDATE";
    }
    return "UNKNOWN";
}

void delay(int num_cycles) {
    sc_time time_to_advance = (num_cycles - 1) * cycle;
    switch (curr_phase()) {
        case phase_t::WRITE:
            time_to_advance += 3 * phase;
            break;
        case phase_t::READ:
            time_to_advance += 2 * phase;
            break;
        case phase_t::UPDATE:
            time_to_advance += phase;
            break;
    }
    wait(time_to_advance);
}

void advance_to_write() {
    switch (curr_phase()) {
        case phase_t::WRITE:
            break;
        case phase_t::READ:
            SC_REPORT_FATAL("timing", "cannot advance to WRITE phase (curr phase == READ)");
            break;
        case phase_t::UPDATE:
            SC_REPORT_FATAL("timing", "cannot advance to WRITE phase (curr phase == UPDATE)");
            break;
    }
}

void advance_to_read() {
    switch (curr_phase()) {
        case phase_t::WRITE:
            wait(phase);
            break;
        case phase_t::READ:
            break;
        case phase_t::UPDATE:
            SC_REPORT_FATAL("timing", "cannot advance to READ phase (curr phase == UPDATE)");
            break;
    }
}

void advance_to_update() {
    switch (curr_phase()) {
        case phase_t::WRITE:
            wait(2 * phase);
            break;
        case phase_t::READ:
            wait(phase);
            break;
        case phase_t::UPDATE:
            break;
    }
}

void assert_on_read() {
    if (curr_phase() != phase_t::READ)
        SC_REPORT_FATAL("timing", "READ phase assertion failed");
}

void assert_on_write() {
    if (curr_phase() != phase_t::WRITE)
        SC_REPORT_FATAL("timing", "WRITE phase assertion failed");
}

struct endl_printer {
    ~endl_printer() { std::cout << std::endl; }
};

#define LOG (endl_printer(), std::cout << name() << " @ " << curr_cycle() << "," << curr_phase_str() << ": ")

#endif //RTCORE_SYSTEMC_UTILITY_HPP