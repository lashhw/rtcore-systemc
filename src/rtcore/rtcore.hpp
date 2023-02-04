#ifndef RTCORE_SYSTEMC_RTCORE_HPP
#define RTCORE_SYSTEMC_RTCORE_HPP

#include "sync_fifo.hpp"
#include "trv_ctrl.hpp"
#include "bbox_ctrl.hpp"
#include "lp.hpp"
#include "hp.hpp"
#include "ist_ctrl.hpp"

SC_MODULE(rtcore) {
    sc_export<blocking<mem_req_t>> p_mem_req;
    sc_export<blocking<mem_resp_t>> p_mem_resp;
    sc_export<blocking<ray_t>> p_shader_ray;
    sc_export<blocking<int>> p_shader_id;
    sc_export<sync_fifo<result_t, fifo_size>> p_shader_result;

    arbiter<mem_req_t, mem_resp_t, 3> m_arbiter;
    trv_ctrl m_trv_ctrl;
    bbox_ctrl m_bbox_ctrl;
    lp m_lp;
    hp m_hp;
    ist_ctrl m_ist_ctrl;

    blocking<mem_req_t> b_arbiter_to_mem;
    blocking<mem_resp_t> b_mem_to_arbiter;
    blocking<mem_req_t> b_trv_ctrl_to_arbiter;
    blocking<mem_resp_t> b_arbiter_to_trv_ctrl;
    blocking<mem_req_t> b_bbox_ctrl_to_arbiter;
    blocking<mem_resp_t> b_arbiter_to_bbox_ctrl;
    blocking<mem_req_t> b_ist_ctrl_to_arbiter;
    blocking<mem_resp_t> b_arbiter_to_ist_ctrl;
    blocking<ray_t> b_shader_to_trv_ctrl;
    blocking<int> b_trv_ctrl_to_shader;

    sync_fifo<bbox_ctrl_req_t, fifo_size> f_trv_ctrl_to_bbox_ctrl;
    sync_fifo<bbox_req_t, fifo_size, num_lp, 1> f_bbox_ctrl_to_lp;
    sync_fifo<trv_ctrl_req_t, fifo_size, 1, num_lp> f_lp_to_trv_ctrl;
    sync_fifo<bbox_req_t, fifo_size, num_hp, 1> f_bbox_ctrl_to_hp;
    sync_fifo<trv_ctrl_req_t, fifo_size, 1, num_hp> f_hp_to_trv_ctrl;
    sync_fifo<result_t, fifo_size> f_trv_ctrl_to_shader;
    sync_fifo<ist_ctrl_req_t, fifo_size> f_trv_ctrl_to_ist_ctrl;
    sync_fifo<trv_ctrl_req_t, fifo_size> f_ist_ctrl_to_trv_ctrl;

    rtcore(sc_module_name mn) : sc_module(mn),
                                m_arbiter("m_arbiter"),
                                m_trv_ctrl("m_trv_ctrl"),
                                m_bbox_ctrl("m_bbox_ctrl"),
                                m_lp("m_lp"),
                                m_hp("m_hp"),
                                m_ist_ctrl("m_ist_ctrl"),
                                b_arbiter_to_mem("b_arbiter_to_mem"),
                                b_mem_to_arbiter("b_mem_to_arbiter"),
                                b_trv_ctrl_to_arbiter("b_trv_ctrl_to_arbiter"),
                                b_arbiter_to_trv_ctrl("b_arbiter_to_trv_ctrl"),
                                b_bbox_ctrl_to_arbiter("b_bbox_ctrl_to_arbiter"),
                                b_arbiter_to_bbox_ctrl("b_arbiter_to_bbox_ctrl"),
                                b_ist_ctrl_to_arbiter("b_ist_ctrl_to_arbiter"),
                                b_arbiter_to_ist_ctrl("b_arbiter_to_ist_ctrl"),
                                b_shader_to_trv_ctrl("b_shader_to_trv_ctrl"),
                                b_trv_ctrl_to_shader("b_trv_ctrl_to_shader"),
                                f_trv_ctrl_to_bbox_ctrl("f_trv_ctrl_to_bbox_ctrl"),
                                f_bbox_ctrl_to_lp("f_bbox_ctrl_to_lp"),
                                f_lp_to_trv_ctrl("f_lp_to_trv_ctrl"),
                                f_bbox_ctrl_to_hp("f_bbox_ctrl_to_hp"),
                                f_hp_to_trv_ctrl("f_hp_to_trv_ctrl"),
                                f_trv_ctrl_to_shader("f_trv_ctrl_to_shader"),
                                f_trv_ctrl_to_ist_ctrl("f_trv_ctrl_to_ist_ctrl"),
                                f_ist_ctrl_to_trv_ctrl("f_ist_ctrl_to_trv_ctrl") {
        // link export
        p_mem_req(b_arbiter_to_mem);
        p_mem_resp(b_mem_to_arbiter);
        p_shader_ray(b_shader_to_trv_ctrl);
        p_shader_id(b_trv_ctrl_to_shader);
        p_shader_result(f_trv_ctrl_to_shader);

        // link arbiter
        m_arbiter.p_master_req(b_arbiter_to_mem);
        m_arbiter.p_master_resp(b_mem_to_arbiter);
        m_arbiter.p_slave_req[0](b_trv_ctrl_to_arbiter);
        m_arbiter.p_slave_resp[0](b_arbiter_to_trv_ctrl);
        m_arbiter.p_slave_req[1](b_bbox_ctrl_to_arbiter);
        m_arbiter.p_slave_resp[1](b_arbiter_to_bbox_ctrl);
        m_arbiter.p_slave_req[2](b_ist_ctrl_to_arbiter);
        m_arbiter.p_slave_resp[2](b_arbiter_to_ist_ctrl);

        // link trv_ctrl
        m_trv_ctrl.p_mem_req(b_trv_ctrl_to_arbiter);
        m_trv_ctrl.p_mem_resp(b_arbiter_to_trv_ctrl);
        m_trv_ctrl.p_shader_ray(b_shader_to_trv_ctrl);
        m_trv_ctrl.p_shader_id(b_trv_ctrl_to_shader);
        m_trv_ctrl.p_shader_result(f_trv_ctrl_to_shader);
        m_trv_ctrl.p_bbox_ctrl(f_trv_ctrl_to_bbox_ctrl);
        m_trv_ctrl.p_lp(f_lp_to_trv_ctrl);
        m_trv_ctrl.p_hp(f_hp_to_trv_ctrl);
        m_trv_ctrl.p_ist_ctrl_out(f_trv_ctrl_to_ist_ctrl);
        m_trv_ctrl.p_ist_ctrl_in(f_ist_ctrl_to_trv_ctrl);

        // link bbox_ctrl
        m_bbox_ctrl.p_mem_req(b_bbox_ctrl_to_arbiter);
        m_bbox_ctrl.p_mem_resp(b_arbiter_to_bbox_ctrl);
        m_bbox_ctrl.p_trv_ctrl(f_trv_ctrl_to_bbox_ctrl);
        m_bbox_ctrl.p_lp(f_bbox_ctrl_to_lp);
        m_bbox_ctrl.p_hp(f_bbox_ctrl_to_hp);

        // link lp
        m_lp.p_bbox_ctrl(f_bbox_ctrl_to_lp);
        m_lp.p_trv_ctrl(f_lp_to_trv_ctrl);

        // link hp
        m_hp.p_bbox_ctrl(f_bbox_ctrl_to_hp);
        m_hp.p_trv_ctrl(f_hp_to_trv_ctrl);

        // link ist_ctrl
        m_ist_ctrl.p_mem_req(b_ist_ctrl_to_arbiter);
        m_ist_ctrl.p_mem_resp(b_arbiter_to_ist_ctrl);
        m_ist_ctrl.p_trv_ctrl_in(f_trv_ctrl_to_ist_ctrl);
        m_ist_ctrl.p_trv_ctrl_out(f_ist_ctrl_to_trv_ctrl);
    }
};

#endif //RTCORE_SYSTEMC_RTCORE_HPP
