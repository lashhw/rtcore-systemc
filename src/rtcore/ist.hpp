#ifndef RTCORE_SYSTEMC_IST_HPP
#define RTCORE_SYSTEMC_IST_HPP

SC_MODULE(ist) {
    sync_fifo_in<ist_req_t> p_ist_ctrl_in;
    sync_fifo_out<ist_ctrl_req_t> p_ist_ctrl_out;
    int num_processed = 0;

    SC_CTOR(ist) {
        SC_THREAD(thread_1);
    }

    ~ist() override {
        std::cout << "ist processed " << num_processed << " triangles" << std::endl;
    }

    void thread_1() {
        std::vector<std::pair<ist_ctrl_req_t, int>> processing;

        while (true) {
            // read: read request
            advance_to_read();
            if (processing.size() < num_ist && p_ist_ctrl_in->readable()) {
                ist_req_t ist_req = p_ist_ctrl_in->read();
                ist_ctrl_req_t ist_ctrl_req = {
                    .ray_and_id = ist_req.ray_and_id,
                    .num_trigs = ist_req.num_trigs - 1,
                    .trig_addr = ist_req.trig_addr + 36,
                    .intersected = ist_req.intersected,
                    .u = ist_req.u,
                    .v = ist_req.v
                };
                bvh::Triangle<float> triangle = to_bvh_triangle(ist_req.trig);
                bvh::Ray<float> ray = to_bvh_ray(ist_req.ray_and_id.ray);
                auto result = triangle.intersect(ray);
                if (result) {
                    ist_ctrl_req.ray_and_id.ray.t_max = result->t;
                    ist_ctrl_req.intersected = true;
                    ist_ctrl_req.u = result->u;
                    ist_ctrl_req.v = result->v;
                }

                processing.emplace_back(ist_ctrl_req, ist_latency);
                num_processed++;
            }

            // update: update counter
            advance_to_update();
            for (auto &p : processing)
                p.second = std::min(p.second, p.second - 1);

            // write: send response
            delay(1);
            advance_to_write();
            for (auto it = processing.begin(); it != processing.end(); it++) {
                if (it->second <= 0) {
                    p_ist_ctrl_out->write(it->first);
                    processing.erase(it);
                    break;
                }
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_HPP
