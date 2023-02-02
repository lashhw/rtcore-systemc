#ifndef RTCORE_SYSTEMC_BBOX_CTRL_HPP
#define RTCORE_SYSTEMC_BBOX_CTRL_HPP

SC_MODULE(bbox_ctrl) {
    blocking_out<to_memory_t> p_memory_req;
    blocking_in<from_memory_t> p_memory_resp;
    blocking_in<to_bbox_ctrl_t> p_trv_ctrl;
    blocking_out<to_bbox_t> p_lp;
    blocking_out<to_bbox_t> p_hp;

    SC_CTOR(bbox_ctrl) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            to_bbox_ctrl_t req = p_trv_ctrl->read();
            std::cout << name() << " @ " << sc_time_stamp() << ": request received!" << std::endl;
            wait(cycle);
        }
    }
};

#endif //RTCORE_SYSTEMC_BBOX_CTRL_HPP
