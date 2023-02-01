#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    blocking_out<to_memory_t> p_memory_req;
    blocking_in<from_memory_t> p_memory_resp;
    sync_fifo_in<to_bbox_ctrl_t> p_trv_ctrl;
    sync_fifo_out<to_bbox_t> p_lp;
    sync_fifo_out<to_bbox_t> p_hp;
    SC_CTOR(bbox_ctrl) {

    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
