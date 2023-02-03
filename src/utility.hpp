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

#endif //RTCORE_SYSTEMC_UTILITY_HPP
