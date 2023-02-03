#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

#include <bvh/triangle.hpp>

SC_MODULE(ist_ctrl) {
    blocking_out<to_mem_t> p_mem_req;
    blocking_in<from_mem_t> p_mem_resp;
    blocking_in<to_ist_ctrl_t> p_trv_ctrl_req;
    blocking_out<to_trv_ctrl_t> p_trv_ctrl_resp;

    SC_CTOR(ist_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            to_ist_ctrl_t req = p_trv_ctrl_req->read();
            wait(cycle);

            to_trv_ctrl_t resp;
            resp.type = to_trv_ctrl_t::IST;
            resp.ist_result.ray_and_id = req.ray_and_id;
            for (int i = 0; i < req.num_trigs; i++) {
                to_mem_t mem_req;
                mem_req.type = to_mem_t::TRIG_IDX;
                mem_req.idx = req.first_trig_idx + i;
                p_mem_req->write(mem_req);
                wait(cycle);

                from_mem_t mem_resp = p_mem_resp->read();
                mem_req.type = to_mem_t::TRIG;
                mem_req.idx = mem_resp.trig_idx;
                p_mem_req->write(mem_req);
                wait(cycle);

                mem_resp = p_mem_resp->read();
                bvh::Triangle<float> triangle = to_bvh_triangle(mem_resp.trig);
                bvh::Ray<float> ray = to_bvh_ray(req.ray_and_id.ray);
                auto result = triangle.intersect(ray);
                if (result) {
                    resp.ist_result.ray_and_id.ray.t_max = result->t;
                    resp.ist_result.intersected = true;
                    resp.ist_result.u = result->u;
                    resp.ist_result.v = result->v;
                }
                wait(cycle);
            }
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
