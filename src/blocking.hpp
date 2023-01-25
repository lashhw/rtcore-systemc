#ifndef RTCORE_SYSTEMC_BLOCKING_HPP
#define RTCORE_SYSTEMC_BLOCKING_HPP

// blocking read interface
template <typename T>
class blocking_in_if : virtual public sc_interface {
public:
    virtual void read(T &) = 0;
    virtual const sc_event &data_written_event() const = 0;
    virtual const bool &data_valid() const = 0;
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
        m_data_valid = false;
    }

    void read(T &val) override {
        while (!m_data_valid)
            wait(m_data_written);
        val = m_data;
        m_data_valid = false;
        m_data_read.notify();
    }

    const sc_event &data_written_event() const override {
        return m_data_written;
    }

    const bool &data_valid() const override {
        return m_data_valid;
    }

    void write(const T &val) override {
        while (m_data_valid)
            wait(m_data_read);
        m_data = val;
        m_data_valid = true;
        m_data_written.notify();
    }

private:
    bool m_data_valid;
    T m_data;
    sc_event m_data_written;
    sc_event m_data_read;
};

#endif //RTCORE_SYSTEMC_BLOCKING_HPP
