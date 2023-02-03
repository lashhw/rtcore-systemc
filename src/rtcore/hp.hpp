#ifndef RTCORE_SYSTEMC_HP_HPP
#define RTCORE_SYSTEMC_HP_HPP

SC_MODULE(hp) {
    sync_fifo_in<to_bbox_t, num_hp> p_bbox_ctrl;
    sync_fifo_out<to_trv_ctrl_t, num_hp> p_trv_ctrl;

    SC_CTOR(hp) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            to_bbox_t req[num_hp];
            p_bbox_ctrl->read(req);
            std::cout << name() << sc_time_stamp() << ": request received!" << std::endl;
            wait(cycle);
        }
    }
};

#endif //RTCORE_SYSTEMC_HP_HPP
