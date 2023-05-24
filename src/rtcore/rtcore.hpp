#ifndef RTCORE_SYSTEMC_RTCORE_HPP
#define RTCORE_SYSTEMC_RTCORE_HPP

#include "../channel/sync_fifo.hpp"
#include "arbiter.hpp"
#include "trv_ctrl.hpp"
#include "bbox_ctrl.hpp"
#include "lp.hpp"
#include "hp.hpp"
#include "ist.hpp"
#include "ist_ctrl.hpp"

SC_MODULE(rtcore) {
    sc_export<blocking<dram_req_t>> p_dram_req;
    sc_export<nonblocking<uint64_t>> p_dram_resp_1;
    sc_export<nonblocking<uint64_t>> p_dram_resp_2;
    sc_export<nonblocking<uint64_t>> p_dram_resp_3;
    sc_export<blocking<ray_t>> p_shader_ray;
    sc_export<blocking<int>> p_shader_id;
    sc_export<sync_fifo<result_t, fifo_size>> p_shader_result;
    sc_port<dram_direct_if> p_dram_direct;

    arbiter<dram_req_t, 3> m_arbiter;
    trv_ctrl m_trv_ctrl;
    bbox_ctrl m_bbox_ctrl;
    lp m_lp;
    hp m_hp;
    ist_ctrl m_ist_ctrl;
    ist m_ist;
    l1c<bbox_req_t> m_bbox_l1c_lp;
    l1c<bbox_req_t> m_bbox_l1c_hp;
    l1c<ist_req_t> m_ist_l1c;

    blocking<dram_req_t> b_arbiter_to_dram;
    blocking<ray_t> b_shader_to_trv_ctrl;
    blocking<int> b_trv_ctrl_to_shader;
    blocking<bbox_l1c_req_t> b_bbox_ctrl_to_l1c_lp;
    blocking<bbox_l1c_req_t> b_bbox_ctrl_to_l1c_hp;
    blocking<ist_l1c_req_t> b_ist_ctrl_to_l1c;

    nonblocking<uint64_t> n_dram_to_bbox_l1c_lp;
    nonblocking<uint64_t> n_dram_to_bbox_l1c_hp;
    nonblocking<uint64_t> n_dram_to_ist_l1c;

    sync_fifo<dram_req_t, fifo_size> f_bbox_l1c_lp_to_arbiter;
    sync_fifo<dram_req_t, fifo_size> f_bbox_l1c_hp_to_arbiter;
    sync_fifo<dram_req_t, fifo_size> f_ist_l1c_to_arbiter;
    sync_fifo<bbox_ctrl_req_t, fifo_size> f_trv_ctrl_to_bbox_ctrl;
    sync_fifo<bbox_req_t, fifo_size, num_lp, 1> f_bbox_ctrl_to_lp;
    sync_fifo<trv_ctrl_req_t, fifo_size, 1, num_lp> f_lp_to_trv_ctrl;
    sync_fifo<bbox_req_t, fifo_size, num_hp, 1> f_bbox_ctrl_to_hp;
    sync_fifo<trv_ctrl_req_t, fifo_size, 1, num_hp> f_hp_to_trv_ctrl;
    sync_fifo<result_t, fifo_size> f_trv_ctrl_to_shader;
    sync_fifo<ist_ctrl_req_t, fifo_size> f_trv_ctrl_to_ist_ctrl;
    sync_fifo<ist_req_t, fifo_size, num_ist, 1> f_ist_ctrl_to_ist;
    sync_fifo<ist_ctrl_req_t, fifo_size, 1, num_ist> f_ist_to_ist_ctrl;
    sync_fifo<trv_ctrl_req_t, fifo_size> f_ist_ctrl_to_trv_ctrl;
    sync_fifo<bbox_req_t, fifo_size> f_l1c_lp_to_bbox_ctrl;
    sync_fifo<bbox_req_t, fifo_size> f_l1c_hp_to_bbox_ctrl;
    sync_fifo<ist_req_t, fifo_size> f_l1c_to_ist_ctrl;

    SC_CTOR(rtcore) : m_arbiter("m_arbiter"),
                      m_trv_ctrl("m_trv_ctrl"),
                      m_bbox_ctrl("m_bbox_ctrl"),
                      m_lp("m_lp"),
                      m_hp("m_hp"),
                      m_ist_ctrl("m_ist_ctrl"),
                      m_ist("m_ist"),
                      m_bbox_l1c_lp("m_bbox_l1c_lp", l1c_lp_num_entries),
                      m_bbox_l1c_hp("m_bbox_l1c_hp", l1c_hp_num_entries),
                      m_ist_l1c("m_ist_l1c", l1c_ist_num_entries),
                      b_arbiter_to_dram("b_arbiter_to_dram"),
                      b_shader_to_trv_ctrl("b_shader_to_trv_ctrl"),
                      b_trv_ctrl_to_shader("b_trv_ctrl_to_shader"),
                      b_bbox_ctrl_to_l1c_lp("b_bbox_ctrl_to_l1c_lp"),
                      b_bbox_ctrl_to_l1c_hp("b_bbox_ctrl_to_l1c_hp"),
                      b_ist_ctrl_to_l1c("b_ist_ctrl_to_l1c"),
                      n_dram_to_bbox_l1c_lp("n_dram_to_bbox_l1c_lp"),
                      n_dram_to_bbox_l1c_hp("n_dram_to_bbox_l1c_hp"),
                      n_dram_to_ist_l1c("n_dram_to_ist_l1c"),
                      f_bbox_l1c_lp_to_arbiter("f_bbox_l1c_lp_to_arbiter"),
                      f_bbox_l1c_hp_to_arbiter("f_bbox_l1c_hp_to_arbiter"),
                      f_ist_l1c_to_arbiter("f_ist_l1c_to_arbiter"),
                      f_trv_ctrl_to_bbox_ctrl("f_trv_ctrl_to_bbox_ctrl"),
                      f_bbox_ctrl_to_lp("f_bbox_ctrl_to_lp"),
                      f_lp_to_trv_ctrl("f_lp_to_trv_ctrl"),
                      f_bbox_ctrl_to_hp("f_bbox_ctrl_to_hp"),
                      f_hp_to_trv_ctrl("f_hp_to_trv_ctrl"),
                      f_trv_ctrl_to_shader("f_trv_ctrl_to_shader"),
                      f_trv_ctrl_to_ist_ctrl("f_trv_ctrl_to_ist_ctrl"),
                      f_ist_ctrl_to_ist("f_ist_ctrl_to_ist"),
                      f_ist_to_ist_ctrl("f_ist_to_ist_ctrl"),
                      f_ist_ctrl_to_trv_ctrl("f_ist_ctrl_to_trv_ctrl"),
                      f_l1c_lp_to_bbox_ctrl("f_l1c_lp_to_bbox_ctrl"),
                      f_l1c_hp_to_bbox_ctrl("f_l1c_hp_to_bbox_ctrl"),
                      f_l1c_to_ist_ctrl("f_l1c_to_ist_ctrl") {
        // link export
        p_dram_req(b_arbiter_to_dram);
        p_dram_resp_1(n_dram_to_bbox_l1c_lp);
        p_dram_resp_2(n_dram_to_bbox_l1c_hp);
        p_dram_resp_3(n_dram_to_ist_l1c);
        p_shader_ray(b_shader_to_trv_ctrl);
        p_shader_id(b_trv_ctrl_to_shader);
        p_shader_result(f_trv_ctrl_to_shader);

        // link arbiter
        m_arbiter.p_slave[0](f_bbox_l1c_lp_to_arbiter);
        m_arbiter.p_slave[1](f_bbox_l1c_hp_to_arbiter);
        m_arbiter.p_slave[2](f_ist_l1c_to_arbiter);
        m_arbiter.p_master(b_arbiter_to_dram);

        // link trv_ctrl
        m_trv_ctrl.p_shader_ray(b_shader_to_trv_ctrl);
        m_trv_ctrl.p_shader_id(b_trv_ctrl_to_shader);
        m_trv_ctrl.p_shader_result(f_trv_ctrl_to_shader);
        m_trv_ctrl.p_bbox_ctrl(f_trv_ctrl_to_bbox_ctrl);
        m_trv_ctrl.p_lp(f_lp_to_trv_ctrl);
        m_trv_ctrl.p_hp(f_hp_to_trv_ctrl);
        m_trv_ctrl.p_ist_ctrl_out(f_trv_ctrl_to_ist_ctrl);
        m_trv_ctrl.p_ist_ctrl_in(f_ist_ctrl_to_trv_ctrl);
        m_trv_ctrl.p_dram_direct(p_dram_direct);

        // link bbox_ctrl
        m_bbox_ctrl.p_l1c_lp_req(b_bbox_ctrl_to_l1c_lp);
        m_bbox_ctrl.p_l1c_lp_resp(f_l1c_lp_to_bbox_ctrl);
        m_bbox_ctrl.p_l1c_hp_req(b_bbox_ctrl_to_l1c_hp);
        m_bbox_ctrl.p_l1c_hp_resp(f_l1c_hp_to_bbox_ctrl);
        m_bbox_ctrl.p_trv_ctrl(f_trv_ctrl_to_bbox_ctrl);
        m_bbox_ctrl.p_lp(f_bbox_ctrl_to_lp);
        m_bbox_ctrl.p_hp(f_bbox_ctrl_to_hp);
        m_bbox_ctrl.p_dram_direct(p_dram_direct);

        // link bbox_l1c_lp
        m_bbox_l1c_lp.p_dram_req(f_bbox_l1c_lp_to_arbiter);
        m_bbox_l1c_lp.p_dram_resp(n_dram_to_bbox_l1c_lp);
        m_bbox_l1c_lp.p_req(b_bbox_ctrl_to_l1c_lp);
        m_bbox_l1c_lp.p_resp(f_l1c_lp_to_bbox_ctrl);

        // link bbox_l1c_hp
        m_bbox_l1c_hp.p_dram_req(f_bbox_l1c_hp_to_arbiter);
        m_bbox_l1c_hp.p_dram_resp(n_dram_to_bbox_l1c_hp);
        m_bbox_l1c_hp.p_req(b_bbox_ctrl_to_l1c_hp);
        m_bbox_l1c_hp.p_resp(f_l1c_hp_to_bbox_ctrl);

        // link lp
        m_lp.p_bbox_ctrl(f_bbox_ctrl_to_lp);
        m_lp.p_trv_ctrl(f_lp_to_trv_ctrl);

        // link hp
        m_hp.p_bbox_ctrl(f_bbox_ctrl_to_hp);
        m_hp.p_trv_ctrl(f_hp_to_trv_ctrl);

        // link ist_ctrl
        m_ist_ctrl.p_l1c_req(b_ist_ctrl_to_l1c);
        m_ist_ctrl.p_l1c_resp(f_l1c_to_ist_ctrl);
        m_ist_ctrl.p_trv_ctrl_in(f_trv_ctrl_to_ist_ctrl);
        m_ist_ctrl.p_trv_ctrl_out(f_ist_ctrl_to_trv_ctrl);
        m_ist_ctrl.p_ist_out(f_ist_ctrl_to_ist);
        m_ist_ctrl.p_ist_in(f_ist_to_ist_ctrl);
        m_ist_ctrl.p_dram_direct(p_dram_direct);

        // link ist_l1c
        m_ist_l1c.p_dram_req(f_ist_l1c_to_arbiter);
        m_ist_l1c.p_dram_resp(n_dram_to_ist_l1c);
        m_ist_l1c.p_req(b_ist_ctrl_to_l1c);
        m_ist_l1c.p_resp(f_l1c_to_ist_ctrl);

        // link ist
        m_ist.p_ist_ctrl_in(f_ist_ctrl_to_ist);
        m_ist.p_ist_ctrl_out(f_ist_to_ist_ctrl);
    }
};

#endif //RTCORE_SYSTEMC_RTCORE_HPP
