#ifndef RTCORE_SYSTEMC_NONBLOCKING_HPP
#define RTCORE_SYSTEMC_NONBLOCKING_HPP

#include "utility.hpp"

template <typename T>
class nonblocking_in_if : virtual public sc_interface {
public:
    virtual bool readable() = 0;
    virtual T read() = 0;
};

template <typename T>
class nonblocking_out_if : virtual public sc_interface {
public:
    virtual void write(const T &val) = 0;
};

template <typename T>
using nonblocking_in = sc_port<nonblocking_in_if<T>>;

template <typename T>
using nonblocking_out = sc_port<nonblocking_out_if<T>>;

template <typename T>
class nonblocking : public sc_channel,
                    public nonblocking_in_if<T>,
                    public nonblocking_out_if<T> {
public:
    SC_CTOR(nonblocking) {
        valid = false;
        has_read = false;

        SC_THREAD(thread_1);
    }

    bool readable() {
        return valid;
    }

    T read() {
        assert_on_read();
        has_read = true;
        return data;
    }

    void write(const T &val) {
        assert_on_write();
        valid = true;
        data = val;
    }

private:
    void thread_1() {
        while (true) {
            advance_to_update();
            if (valid && !has_read)
                SC_REPORT_FATAL("timing", "data does not read within one cycle");
            valid = false;
            has_read = false;
            delay(1);
        }
    }

    bool valid;
    bool has_read;
    T data;
};

#endif //RTCORE_SYSTEMC_NONBLOCKING_HPP
