#ifndef RTCORE_SYSTEMC_IST_HPP
#define RTCORE_SYSTEMC_IST_HPP

SC_MODULE(ist) {
    sync_fifo_in<ist_req_t, num_ist> p_ist_ctrl_in;
    sync_fifo_out<ist_resp_t, num_ist> p_ist_ctrl_out;

    SC_CTOR(ist) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            ist_req_t req[num_ist];
            p_ist_ctrl_in->read(req);

            wait(ist_latency * cycle);
            ist_resp_t resp[num_ist];
            for (int i = 0; i < num_ist; i++) {
                resp[i].ray_and_id = req[i].ray_and_id;
                resp[i].intersected = false;
                bvh::Triangle<float> triangle = to_bvh_triangle(req[i].trig);
                bvh::Ray<float> ray = to_bvh_ray(req[i].ray_and_id.ray);
                auto result = triangle.intersect(ray);
                if (result) {
                    resp[i].ray_and_id.ray.t_max = result->t;
                    resp[i].intersected = true;
                    resp[i].u = result->u;
                    resp[i].v = result->v;
                }
            }
            p_ist_ctrl_out->write(resp);
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_HPP
