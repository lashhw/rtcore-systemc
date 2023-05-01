#ifndef RTCORE_SYSTEMC_IST_HPP
#define RTCORE_SYSTEMC_IST_HPP

SC_MODULE(ist) {
    sync_fifo_in<ist_req_t, num_ist> p_ist_ctrl_in;
    sync_fifo_out<ist_ctrl_req_t, num_ist> p_ist_ctrl_out;
    int num_processed = 0;

    SC_CTOR(ist) {
        SC_THREAD(thread_1);
    }

    ~ist() override {
        std::cout << "ist processed " << num_processed << " triangles" << std::endl;
    }

    void thread_1() {
        while (true) {
            ist_req_t ist_req[num_ist];
            p_ist_ctrl_in->read(ist_req);
            delay(ist_latency);
            ist_ctrl_req_t ist_ctrl_req[num_ist];
            for (int i = 0; i < num_ist; i++) {
                ist_ctrl_req[i] = {
                    .ray_and_id = ist_req[i].ray_and_id,
                    .num_trigs = ist_req[i].num_trigs - 1,
                    .trig_addr = ist_req[i].trig_addr + 36,
                    .intersected = ist_req[i].intersected,
                    .u = ist_req[i].u,
                    .v = ist_req[i].v
                };
                bvh::Triangle<float> triangle = to_bvh_triangle(ist_req[i].trig);
                bvh::Ray<float> ray = to_bvh_ray(ist_req[i].ray_and_id.ray);
                auto result = triangle.intersect(ray);
                if (result) {
                    ist_ctrl_req[i].ray_and_id.ray.t_max = result->t;
                    ist_ctrl_req[i].intersected = true;
                    ist_ctrl_req[i].u = result->u;
                    ist_ctrl_req[i].v = result->v;
                }
            }
            num_processed += num_ist;
            p_ist_ctrl_out->write(ist_ctrl_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_HPP
