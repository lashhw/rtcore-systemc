#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

#include <bvh/triangle.hpp>

SC_MODULE(ist_ctrl) {
    blocking_out<mem_req_t> p_mem_req;
    blocking_in<mem_resp_t> p_mem_resp;
    blocking_in<ist_ctrl_req_t> p_trv_ctrl_in;
    blocking_out<trv_ctrl_req_t> p_trv_ctrl_out;

    SC_CTOR(ist_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            wait(cycle);
            ist_ctrl_req_t ist_ctrl_req = p_trv_ctrl_in->read();
            trv_ctrl_req_t trv_ctrl_req;
            trv_ctrl_req.type = trv_ctrl_req_t::IST;
            trv_ctrl_req.ist.ray_and_id = ist_ctrl_req.ray_and_id;
            trv_ctrl_req.ist.intersected = false;

            for (int i = 0; i < ist_ctrl_req.num_trigs; i++) {
                wait(cycle);
                mem_req_t mem_req;
                mem_req.type = mem_req_t::TRIG_IDX;
                mem_req.idx = ist_ctrl_req.first_trig_idx + i;
                p_mem_req->write(mem_req);

                wait(cycle);
                mem_resp_t mem_resp = p_mem_resp->read();
                mem_req.type = mem_req_t::TRIG;
                mem_req.idx = mem_resp.trig_idx;
                p_mem_req->write(mem_req);

                wait(cycle);
                mem_resp = p_mem_resp->read();
                bvh::Triangle<float> triangle = to_bvh_triangle(mem_resp.trig);
                bvh::Ray<float> ray = to_bvh_ray(trv_ctrl_req.ist.ray_and_id.ray);
                auto result = triangle.intersect(ray);
                if (result) {
                    trv_ctrl_req.ist.ray_and_id.ray.t_max = result->t;
                    trv_ctrl_req.ist.intersected = true;
                    trv_ctrl_req.ist.u = result->u;
                    trv_ctrl_req.ist.v = result->v;
                }
            }

            wait(cycle);
            p_trv_ctrl_out->write(trv_ctrl_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
