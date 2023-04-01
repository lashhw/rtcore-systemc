#ifndef RTCORE_SYSTEMC_BLOCKING_HPP
#define RTCORE_SYSTEMC_BLOCKING_HPP

#include "utility.hpp"

// blocking read interface
template <typename T>
class blocking_in_if : virtual public sc_interface {
public:
    virtual bool nb_readable() const = 0;
    virtual T read() = 0;
};

// blocking write interface
template <typename T>
class blocking_out_if : virtual public sc_interface {
public:
    virtual void write(const T &) = 0;
};

// alias for sc_port<blocking_in_if<T>>
template <typename T>
using blocking_in = sc_port<blocking_in_if<T>>;

// alias for sc_port<blocking_out_if<T>>
template <typename T>
using blocking_out = sc_port<blocking_out_if<T>>;

// channel that supports blocking read/write
template <typename T>
class blocking : public sc_channel,
                 public blocking_in_if<T>,
                 public blocking_out_if<T> {
public:
    SC_CTOR(blocking) {
        data_read = false;
        data_written = false;
    }

    ~blocking() override {
        std::string mn = name();
        if (data_read)
            SC_REPORT_WARNING("communication", ("unfinished read in " + mn).c_str());
        if (data_written)
            SC_REPORT_WARNING("communication", ("unfinished write in " + mn).c_str());
    }

    bool nb_readable() const override {
        assert_on_read();
        return data_written;
    }

    T read() override {
        advance_to_read();
        data_read_event.notify();
        if (data_written) {
            data_written = false;
        } else {
            data_read = true;
            wait(data_written_event);
            advance_to_read();
        }
        return data;
    }

    void write(const T &val) override {
        advance_to_write();
        data = val;
        data_written_event.notify();
        if (data_read) {
            data_read = false;
        } else {
            data_written = true;
            wait(data_read_event);
        }
    }

private:
    T data;
    bool data_read;
    sc_event data_read_event;
    bool data_written;
    sc_event data_written_event;
};

#endif //RTCORE_SYSTEMC_BLOCKING_HPP
