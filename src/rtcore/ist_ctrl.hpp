#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

#include <bvh/triangle.hpp>
#include "ist.hpp"

SC_MODULE(ist_ctrl) {
    blocking_out<mem_req_t> p_mem_req;
    blocking_in<mem_resp_t> p_mem_resp;
    blocking_in<ist_ctrl_req_t> p_trv_ctrl_in;
    blocking_out<trv_ctrl_req_t> p_trv_ctrl_out;
    blocking_out<ist_req_t> p_ist_out;
    blocking_in<ist_resp_t> p_ist_in;

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
                ist_req_t ist_req;
                ist_req.ray_and_id = trv_ctrl_req.ist.ray_and_id;
                ist_req.trig = mem_resp.trig;
                p_ist_out->write(ist_req);

                wait(cycle);
                ist_resp_t ist_resp = p_ist_in->read();
                trv_ctrl_req.ist.ray_and_id.ray = ist_resp.ray_and_id.ray;
                if (ist_resp.intersected) {
                    trv_ctrl_req.ist.intersected = true;
                    trv_ctrl_req.ist.u = ist_resp.u;
                    trv_ctrl_req.ist.v = ist_resp.v;
                }
            }

            wait(cycle);
            p_trv_ctrl_out->write(trv_ctrl_req);
        }
    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
