#ifndef RTCORE_SYSTEMC_SHADER_HPP
#define RTCORE_SYSTEMC_SHADER_HPP

#include "misc.hpp"

SC_MODULE(shader) {
    blocking_out<ray_t> p_ray;
    blocking_in<int> p_id;
    blocking_in<to_shader_t> p_result;

    SC_HAS_PROCESS(shader);
    shader(sc_module_name mn, const char *ray_queries_path) : sc_module(mn),
                                                              ray_queries_path(ray_queries_path) {
        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        std::ifstream ray_queries_file(ray_queries_path, std::ios::in | std::ios::binary);
        float r[7];
        while (ray_queries_file.read(reinterpret_cast<char*>(&r), 7*sizeof(float))) {
            ray_t ray{r[0], r[1], r[2], r[3], r[4], r[5], 0.0f, r[6]};
            p_ray->write(ray);
            int id = p_id->read();
            processing_ray[id] = ray;
            std::cout << name() << "@" << sc_time_stamp() << ": ray sent, id=" << id << std::endl;
        }
    }

    void thread_2() {
        while (true) {
            to_shader_t result = p_result->read();
            std::cout << name() << "@" << sc_time_stamp() << ": result received, id=" << result.id << std::endl;
        }
    }

    std::string ray_queries_path;
    ray_t processing_ray[num_working_rays];
};

#endif //RTCORE_SYSTEMC_SHADER_HPP
