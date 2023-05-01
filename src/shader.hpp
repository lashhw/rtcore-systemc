#ifndef RTCORE_SYSTEMC_SHADER_HPP
#define RTCORE_SYSTEMC_SHADER_HPP

SC_MODULE(shader) {
    blocking_out<ray_t> p_rtcore_ray;
    blocking_in<int> p_rtcore_id;
    blocking_in<result_t> p_rtcore_result;
    sc_port<dram_direct_if> p_dram_direct;

    std::ifstream ray_queries_file;
    struct {
        bool intersected;
        float t;
        float u;
        float v;
    } answer[num_working_rays];

    SC_HAS_PROCESS(shader);
    shader(const sc_module_name &mn, const char *ray_queries_path) : sc_module(mn) {
        ray_queries_file.open(ray_queries_path, std::ios::in | std::ios::binary);
        if (!ray_queries_file)
            SC_REPORT_FATAL("file", "failed to open ray_queries");

        SC_THREAD(thread_1);
        SC_THREAD(thread_2);
    }

    void thread_1() {
        float r[7];
        while (ray_queries_file.read(reinterpret_cast<char*>(&r), 7*sizeof(float))) {
            ray_t ray{r[0], r[1], r[2], r[3], r[4], r[5], 0.0f, r[6]};
            p_rtcore_ray->write(ray);
            int id = p_rtcore_id->read();
            p_dram_direct->direct_traverse(ray, answer[id].intersected, answer[id].t, answer[id].u, answer[id].v);
            LOG << "ray sent, id=" << id;
            delay(1);
        }
        sc_stop();
    }

    void thread_2() {
        while (true) {
            result_t result = p_rtcore_result->read();
            LOG << "result received, id=" << result.id;
            if (result.intersected)
                LOG << "  [result] intersected " << result.t << " " << result.u << " " << result.v;
            else
                LOG << "  [result] not intersected";
            if (answer[result.id].intersected)
                LOG << "  [answer] intersected " << answer[result.id].t << " " << answer[result.id].u << " " << answer[result.id].v;
            else
                LOG << "  [answer] not intersected";
            if (result.intersected) {
                sc_assert(answer[result.id].intersected);
                sc_assert(result.t == answer[result.id].t);
                sc_assert(result.u == answer[result.id].u);
                sc_assert(result.v == answer[result.id].v);
            } else {
                sc_assert(!answer[result.id].intersected);
            }
            delay(1);
        }
    }
};

#endif //RTCORE_SYSTEMC_SHADER_HPP
