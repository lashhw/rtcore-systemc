#ifndef RTCORE_SYSTEMC_FORK_HPP
#define RTCORE_SYSTEMC_FORK_HPP

template <typename T, int num_masters>
SC_MODULE(fork) {
    blocking_in<T> p_slave;
    blocking_out<T> p_master[num_masters];

    SC_CTOR(fork) {
        SC_THREAD(thread_1);
    }

    void thread_1() {
        while (true) {
            T data = p_slave->peek();
            for (int i = 0; i < num_masters; i++)
                p_master[i]->write(data);
            p_slave->read();
        }
    }
};

#endif //RTCORE_SYSTEMC_FORK_HPP
