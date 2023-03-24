#ifndef RTCORE_SYSTEMC_UTILITY_HPP
#define RTCORE_SYSTEMC_UTILITY_HPP

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

#define ASSERT_ON_NEGEDGE()                                            \
    if (sc_time_stamp().value() % cycle.value() != half_cycle.value()) \
        SC_REPORT_FATAL(name(), "negedge assertion failed")

#define ASSERT_ON_POSEDGE()                                 \
    if (sc_time_stamp().value() % cycle.value() != 0)       \
        SC_REPORT_FATAL(name(), "posedge assertion failed")

#define ADVANCE_TO_NEGEDGE()                                  \
    {                                                         \
        auto phase = sc_time_stamp().value() % cycle.value(); \
        if (phase == 0)                                       \
            wait(half_cycle);                                 \
        else                                                  \
            ASSERT_ON_NEGEDGE();                              \
    }

#define ADVANCE_TO_POSEDGE()                                  \
    {                                                         \
        auto phase = sc_time_stamp().value() % cycle.value(); \
        if (phase == half_cycle.value())                      \
            wait(half_cycle);                                 \
        else                                                  \
            ASSERT_ON_POSEDGE();                              \
    }

#define SHOULD_NOT_BE_BLOCKED( expr )                         \
    {                                                         \
        sc_time time_before_write = sc_time_stamp();          \
        expr;                                                 \
        if (sc_time_stamp() - time_before_write >= cycle)     \
            SC_REPORT_FATAL(name(), "should not be blocked"); \
    }

#endif //RTCORE_SYSTEMC_UTILITY_HPP
