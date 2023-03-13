#ifndef RTCORE_SYSTEMC_SHADER_HPP
#define RTCORE_SYSTEMC_SHADER_HPP

#include "params.hpp"

SC_MODULE(shader) {
    blocking_out<ray_t> p_rtcore_ray;
    blocking_in<int> p_rtcore_id;
    blocking_in<result_t> p_rtcore_result;

    SC_HAS_PROCESS(shader);
    shader(const sc_module_name &mn, const char *ray_queries_path, dram &mem_ref) : sc_module(mn),
                                                                                    mem_ref(mem_ref) {
        ray_queries_file.open(ray_queries_path, std::ios::in | std::ios::binary);
        sc_assert(ray_queries_file.good());

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        float r[7];
        while (ray_queries_file.read(reinterpret_cast<char*>(&r), 7*sizeof(float))) {
            wait(cycle);
            ray_t ray{r[0], r[1], r[2], r[3], r[4], r[5], 0.0f, r[6]};
            p_rtcore_ray->write(ray);
            int id = p_rtcore_id->read();
            mem_ref.direct_traverse(ray, answer[id].intersected, answer[id].t, answer[id].u, answer[id].v);
            std::cout << name() << " @ " << sc_time_stamp() << ": ray sent, id=" << id << std::endl;
        }
        std::cout << "TEST COMPLETED" << std::endl;
    }

    void thread_2() {
        while (true) {
            wait(cycle);
            result_t result = p_rtcore_result->read();
            if (result.intersected) {
                sc_assert(answer[result.id].intersected);
                sc_assert(result.t == answer[result.id].t);
                sc_assert(result.u == answer[result.id].u);
                sc_assert(result.v == answer[result.id].v);
            } else {
                sc_assert(!answer[result.id].intersected);
            }
            std::cout << name() << " @ " << sc_time_stamp() << ": result received, id=" << result.id << std::endl;
        }
    }

    dram &mem_ref;
    std::ifstream ray_queries_file;
    struct {
        bool intersected;
        float t;
        float u;
        float v;
    } answer[num_working_rays];
};

#endif //RTCORE_SYSTEMC_SHADER_HPP
