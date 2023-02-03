#ifndef RTCORE_SYSTEMC_LP_HPP
#define RTCORE_SYSTEMC_LP_HPP

SC_MODULE(lp) {
    sync_fifo_in<bbox_req_t, num_lp> p_bbox_ctrl;
    sync_fifo_out<trv_ctrl_req_t, num_lp> p_trv_ctrl;

    SC_CTOR(lp) {

    }
};

#endif //RTCORE_SYSTEMC_LP_HPP
