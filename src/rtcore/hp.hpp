#ifndef RTCORE_SYSTEMC_HP_HPP
#define RTCORE_SYSTEMC_HP_HPP

SC_MODULE(hp) {
    sync_fifo_in<to_bbox_t, num_hp> p_bbox_ctrl;
    sync_fifo_out<to_trv_ctrl_t, num_hp> p_trv_ctrl;

    SC_CTOR(hp) {

    }
};

#endif //RTCORE_SYSTEMC_HP_HPP
