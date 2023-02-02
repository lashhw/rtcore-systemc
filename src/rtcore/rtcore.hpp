#ifndef RTCORE_SYSTEMC_RTCORE_HPP
#define RTCORE_SYSTEMC_RTCORE_HPP

#include "sync_fifo.hpp"
#include "trv_ctrl.hpp"
#include "bbox_ctrl.hpp"
#include "lp.hpp"
#include "hp.hpp"
#include "ist_ctrl.hpp"

SC_MODULE(rtcore) {
    sc_export<blocking<bvh::Ray<float>>> p_ray;
    sc_export<blocking<int>> p_id;
    sc_export<blocking<to_shader_t>> p_result;
    sc_export<blocking<to_memory_t>> p_mem_req;
    sc_export<blocking<from_memory_t>> p_mem_resp;

    arbiter<to_memory_t, from_memory_t, 3> m_arbiter;
    trv_ctrl m_trv_ctrl;
    bbox_ctrl m_bbox_ctrl;
    lp m_lp;
    hp m_hp;
    ist_ctrl m_ist_ctrl;

    blocking<to_memory_t> b_arbiter_to_memory;
    blocking<from_memory_t> b_memory_to_arbiter;
    blocking<to_memory_t> b_trv_ctrl_to_arbiter;
    blocking<from_memory_t> b_arbiter_to_trv_ctrl;
    blocking<to_memory_t> b_bbox_ctrl_to_arbiter;
    blocking<from_memory_t> b_arbiter_to_bbox_ctrl;
    blocking<to_memory_t> b_ist_ctrl_to_arbiter;
    blocking<from_memory_t> b_arbiter_to_ist_ctrl;
    blocking<to_trv_ctrl_t> b_shader_to_trv_ctrl;
    blocking<int> b_trv_ctrl_to_shader;

    sync_fifo<to_bbox_ctrl_t, fifo_size> f_trv_ctrl_to_bbox_ctrl;
    sync_fifo<to_bbox_t, fifo_size, num_lp, 1> f_bbox_ctrl_to_lp;
    sync_fifo<to_trv_ctrl_t, fifo_size, 1, num_lp> f_lp_to_trv_ctrl;
    sync_fifo<to_bbox_t, fifo_size, num_hp, 1> f_bbox_ctrl_to_hp;
    sync_fifo<to_trv_ctrl_t, fifo_size, 1, num_hp> f_hp_to_trv_ctrl;
    sync_fifo<to_shader_t, fifo_size> f_trv_ctrl_to_shader;
    sync_fifo<to_ist_ctrl_t, fifo_size> f_trv_ctrl_to_ist_ctrl;
    sync_fifo<to_trv_ctrl_t, fifo_size> f_ist_ctrl_to_trv_ctrl;

    rtcore(sc_module_name mn) : sc_module(mn),
                                m_arbiter("m_arbiter"),
                                m_trv_ctrl("m_trv_ctrl"),
                                m_bbox_ctrl("m_bbox_ctrl"),
                                m_lp("m_lp"),
                                m_hp("m_hp"),
                                m_ist_ctrl("m_ist_ctrl"),
                                b_arbiter_to_memory("b_arbiter_to_memory"),
                                b_memory_to_arbiter("b_memory_to_arbiter"),
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
        // link arbiter
        m_arbiter.master_to(b_arbiter_to_memory);
        m_arbiter.master_from(b_memory_to_arbiter);
        m_arbiter.slave_from[0](b_trv_ctrl_to_arbiter);
        m_arbiter.slave_to[0](b_arbiter_to_trv_ctrl);
        m_arbiter.slave_from[1](b_bbox_ctrl_to_arbiter);
        m_arbiter.slave_to[1](b_arbiter_to_bbox_ctrl);
        m_arbiter.slave_from[2](b_ist_ctrl_to_arbiter);
        m_arbiter.slave_to[2](b_arbiter_to_ist_ctrl);

        // link trv_ctrl
        m_trv_ctrl.p_memory_req(b_trv_ctrl_to_arbiter);
        m_trv_ctrl.p_memory_resp(b_arbiter_to_trv_ctrl);
        m_trv_ctrl.p_shader_req(b_shader_to_trv_ctrl);
        m_trv_ctrl.p_shader_resp(b_trv_ctrl_to_shader);
        m_trv_ctrl.p_result(f_trv_ctrl_to_shader);
        m_trv_ctrl.p_bbox_ctrl(f_trv_ctrl_to_bbox_ctrl);
        m_trv_ctrl.p_lp(f_lp_to_trv_ctrl);
        m_trv_ctrl.p_hp(f_hp_to_trv_ctrl);
        m_trv_ctrl.p_ist_ctrl(f_trv_ctrl_to_ist_ctrl);

        // link bbox_ctrl
        m_bbox_ctrl.p_memory_req(b_bbox_ctrl_to_arbiter);
        m_bbox_ctrl.p_memory_resp(b_arbiter_to_bbox_ctrl);
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
        m_ist_ctrl.p_trv_ctrl_in(f_trv_ctrl_to_ist_ctrl);
        m_ist_ctrl.p_trv_ctrl_out(f_ist_ctrl_to_trv_ctrl);
    }
};

#endif //RTCORE_SYSTEMC_RTCORE_HPP
