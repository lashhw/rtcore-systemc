#ifndef RTCORE_SYSTEMC_BLOCKING_HPP
#define RTCORE_SYSTEMC_BLOCKING_HPP

// blocking read interface
template <typename T>
class blocking_in_if : virtual public sc_interface {
public:
    virtual void read(T &) = 0;
    virtual const sc_event &data_written_event() const = 0;
    virtual const bool &data_written() const = 0;
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
        waiting = false;
        m_data_written = false;
    }

    void read(T &val) override {
        block();
        val = data;
    }

    const sc_event &data_written_event() const override {
        return m_data_written_event;
    }

    const bool &data_written() const override {
        return m_data_written;
    }

    void write(const T &val) override {
        data = val;
        m_data_written = true;
        m_data_written_event.notify();
        block();
        m_data_written = false;
    }

private:
    void block() {
        if (!waiting) {
            waiting = true;
            wait(trigger);
            waiting = false;
        } else {
            trigger.notify();
        }
    }

    bool waiting;
    sc_event trigger;
    T data;

    bool m_data_written;
    sc_event m_data_written_event;
};

#endif //RTCORE_SYSTEMC_BLOCKING_HPP
