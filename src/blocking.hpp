#ifndef RTCORE_SYSTEMC_BLOCKING_HPP
#define RTCORE_SYSTEMC_BLOCKING_HPP

// blocking read interface
template <typename T>
class blocking_in_if : virtual public sc_interface {
public:
    virtual const sc_event &data_written_event() const = 0;
    virtual bool nb_readable() const = 0;
    virtual T read() = 0;
    virtual T peek() = 0;
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
        m_data_read = false;
        m_data_written = false;
    }

    ~blocking() override {
        if (m_data_read)
            std::cerr << "unfinished read: " << name() << std::endl;
        if (m_data_written)
            std::cerr << "unfinished write: " << name() << std::endl;
    }

    const sc_event &data_written_event() const override {
        return m_data_written_event;
    }

    bool nb_readable() const override {
        return m_data_written;
    }

    T read() override {
        m_data_read_event.notify();
        if (m_data_written) {
            m_data_written = false;
        } else {
            m_data_read = true;
            wait(m_data_written_event);
        }
        return data;
    }

    T peek() override {
        if (!m_data_written)
            wait(m_data_written_event);
        return data;
    }

    void write(const T &val) override {
        data = val;
        m_data_written_event.notify();
        if (m_data_read) {
            m_data_read = false;
        } else {
            m_data_written = true;
            wait(m_data_read_event);
        }
    }

private:
    T data;
    bool m_data_read;
    sc_event m_data_read_event;
    bool m_data_written;
    sc_event m_data_written_event;
};

#endif //RTCORE_SYSTEMC_BLOCKING_HPP
