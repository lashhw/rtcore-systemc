#ifndef RTCORE_SYSTEMC_SHADER_HPP
#define RTCORE_SYSTEMC_SHADER_HPP

#include "params.hpp"

SC_MODULE(shader) {
    blocking_out<ray_t> p_rtcore_ray;
    blocking_in<int> p_rtcore_id;
    blocking_in<result_t> p_rtcore_result;

    SC_HAS_PROCESS(shader);
    shader(sc_module_name mn, const char *ray_queries_path) : sc_module(mn) {
        ray_queries_file.open(ray_queries_path, std::ios::in | std::ios::binary);
        sc_assert(ray_queries_file.good());

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        float r[7];
        while (ray_queries_file.read(reinterpret_cast<char*>(&r), 7*sizeof(float))) {
            ray_t ray{r[0], r[1], r[2], r[3], r[4], r[5], 0.0f, r[6]};
            p_rtcore_ray->write(ray);
            int id = p_rtcore_id->read();
            std::cout << name() << "@" << sc_time_stamp() << ": ray sent, id=" << id << std::endl;
            wait(cycle);
        }
    }

    void thread_2() {
        while (true) {
            result_t result = p_rtcore_result->read();
            std::cout << name() << " @ " << sc_time_stamp() << ": result received, id=" << result.id << std::endl;
            wait(cycle);
        }
    }

    std::ifstream ray_queries_file;
};

#endif //RTCORE_SYSTEMC_SHADER_HPP
