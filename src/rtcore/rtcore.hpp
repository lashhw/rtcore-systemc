#ifndef RTCORE_SYSTEMC_RTCORE_HPP
#define RTCORE_SYSTEMC_RTCORE_HPP

#include "sync_fifo.hpp"
#include "trv_ctrl.hpp"
#include "bbox_ctrl.hpp"
#include "lp.hpp"
#include "hp.hpp"
#include "ist_ctrl.hpp"
#include "ist.hpp"

SC_MODULE(rtcore) {
    blocking_in<bvh::Ray<float>> p_ray;
    blocking_out<int> p_id;
    blocking_out<to_shader_t> p_result;

    blocking<to_trv_ctrl_t> b_shader_to_trv_ctrl;
    blocking<int> b_trv_ctrl_to_shader;
    blocking<to_memory_t> b_trv_ctrl_to_arbiter;
    blocking<from_memory_t> b_arbiter_to_trv_ctrl;
    blocking<to_memory_t> b_bbox_ctrl_to_arbiter;
    blocking<from_memory_t> b_arbiter_to_bbox_ctrl;

    sync_fifo<to_bbox_ctrl_t, fifo_size> f_trv_ctrl_to_bbox_ctrl;
    sync_fifo<to_bbox_ctrl_t, fifo_size, num_lp, 1> f_bbox_ctrl_to_lp;
    sync_fifo<to_bbox_ctrl_t, fifo_size, 1, num_lp> f_lp_to_trv_ctrl;
    sync_fifo<to_bbox_ctrl_t, fifo_size, num_hp, 1> f_bbox_ctrl_to_hp;
    sync_fifo<to_bbox_ctrl_t, fifo_size, 1, num_hp> f_hp_to_trv_ctrl;
    sync_fifo<to_bbox_ctrl_t, fifo_size> f_shader_to_trv_ctrl;
    sync_fifo<to_bbox_ctrl_t, fifo_size> f_trv_ctrl_to_ist_ctrl;
    sync_fifo<to_bbox_ctrl_t, fifo_size> f_ist_ctrl_to_ist;

    arbiter<blocking_in, blocking_out, to_memory_t, from_memory_t, 3> m_arbiter;
    trv_ctrl m_trv_ctrl;
    bbox_ctrl m_bbox_ctrl;
    lp m_lp;
    hp m_hp;
    ist_ctrl m_ist_ctrl;
    ist m_ist;

    rtcore(sc_module_name mn) : sc_module(mn),
                                b_shader_to_trv_ctrl("b_shader_to_trv_ctrl"),
                                b_trv_ctrl_to_shader("b_trv_ctrl_to_shader"),
                                b_trv_ctrl_to_arbiter("b_trv_ctrl_to_arbiter"),
                                b_arbiter_to_trv_ctrl("b_arbiter_to_trv_ctrl"),
                                b_bbox_ctrl_to_arbiter("b_bbox_ctrl_to_arbiter"),
                                b_arbiter_to_bbox_ctrl("b_arbiter_to_bbox_ctrl"),
                                f_trv_ctrl_to_bbox_ctrl("f_trv_ctrl_to_bbox_ctrl"),
                                f_bbox_ctrl_to_lp("f_bbox_ctrl_to_lp"),
                                f_lp_to_trv_ctrl("f_lp_to_trv_ctrl"),
                                f_bbox_ctrl_to_hp("f_bbox_ctrl_to_hp"),
                                f_hp_to_trv_ctrl("f_hp_to_trv_ctrl"),
                                f_shader_to_trv_ctrl("f_shader_to_trv_ctrl"),
                                f_trv_ctrl_to_ist_ctrl("f_trv_ctrl_to_ist_ctrl"),
                                f_ist_ctrl_to_ist("f_ist_ctrl_to_ist"),
                                m_arbiter("m_arbiter"),
                                m_trv_ctrl("m_trv_ctrl"),
                                m_bbox_ctrl("m_bbox_ctrl"),
                                m_lp("m_lp"),
                                m_hp("m_hp"),
                                m_ist_ctrl("m_ist_ctrl"),
                                m_ist("m_ist") {

    }
};

#endif //RTCORE_SYSTEMC_RTCORE_HPP
