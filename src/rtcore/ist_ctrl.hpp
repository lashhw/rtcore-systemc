#ifndef RTCORE_SYSTEMC_IST_CTRL_HPP
#define RTCORE_SYSTEMC_IST_CTRL_HPP

SC_MODULE(ist_ctrl) {
    blocking_out<to_mem_t> p_mem_req;
    blocking_in<from_mem_t> p_mem_resp;
    blocking_in<to_ist_ctrl_t> p_trv_ctrl_in;
    blocking_out<to_trv_ctrl_t> p_trv_ctrl_out;

    SC_CTOR(ist_ctrl) {

    }
};

#endif //RTCORE_SYSTEMC_IST_CTRL_HPP
