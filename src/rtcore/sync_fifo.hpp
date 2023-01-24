#ifndef RTCORE_SYSTEMC_SYNC_FIFO_HPP
#define RTCORE_SYSTEMC_SYNC_FIFO_HPP

#include <array>

// base input interface for sync_fifo
class sync_fifo_base_in_if : virtual public sc_interface {
public:
    virtual const sc_event & write_updated_event() = 0;
    virtual const int & num_elements() = 0;
};

// input interface for sync_fifo
template <typename T, int num_read = 1>
class sync_fifo_in_if : virtual public sync_fifo_base_in_if {
public:
    virtual void read(std::array<T, num_read> &) = 0;
};

// partial template specialization for sync_fifo_in_if when num_read = 1
template <typename T>
class sync_fifo_in_if<T, 1> : virtual public sync_fifo_base_in_if {
public:
    virtual void read(T &) = 0;
};

// output interface for sync_fifo
template <typename T, int num_write = 1>
class sync_fifo_out_if : virtual public sc_interface {
public:
    virtual void write(const std::array<T, num_write> &) = 0;
};

// partial template specialization for sync_fifo_out_if when num_write = 1
template <typename T>
class sync_fifo_out_if<T, 1> : virtual public sc_interface {
public:
    virtual void write(const T &) = 0;
};

// alias for sc_port<sync_fifo_in_if<T, num_read>>
template <typename T, int num_read = 1>
using sync_fifo_in = sc_port<sync_fifo_in_if<T, num_read>>;

// alias for sc_port<sync_fifo_out_if<T, num_write>>
template <typename T, int num_write = 1>
using sync_fifo_out = sc_port<sync_fifo_out_if<T, num_write>>;

// base channel for sync_fifo
template<typename T, int max_size, int num_read, int num_write>
class sync_fifo_base : public sc_channel,
                       virtual public sync_fifo_base_in_if {
public:
    SC_CTOR(sync_fifo_base) {
        curr = 0;
        size = 0;
        read_granted_flag = false;
        write_granted_flag = false;
        SC_THREAD(main_thread);
    }

    const sc_event & write_updated_event() override {
        return write_updated;
    }

    const int & num_elements() override {
        return size;
    }

protected:
    void main_thread() {
        while (true) {
            wait(read_granted | write_granted);
            wait(0.5, SC_NS);

            // read
            if (read_granted_flag) {
                curr = (curr + num_read) % max_size;
                size -= num_read;
                read_granted_flag = false;
                read_updated.notify(0.5, SC_NS);
            }

            // write
            if (write_granted_flag) {
                for (int i = 0; i < num_write; i++) {
                    data[(curr+size)%max_size] = write_data[i];
                    size++;
                }
                write_granted_flag = false;
                write_updated.notify(0.5, SC_NS);
            }

            // print
            std::cout << "sync_fifo @ " << sc_time_stamp() << ": ";
            for (int i = 0; i < size; i++)
                std::cout << data[(curr+i)%max_size] << " ";
            std::cout << std::endl;
        }
    }

    T data[max_size];
    int curr;
    int size;

    bool read_granted_flag;
    sc_event read_granted;
    sc_event read_updated;

    T write_data[num_write];
    bool write_granted_flag;
    sc_event write_granted;
    sc_event write_updated;
};

// sync_fifo
template<typename T, int max_size, int num_read = 1, int num_write = 1>
class sync_fifo : public sync_fifo_base<T, max_size, num_read, num_write>,
                  public sync_fifo_in_if<T, num_read>,
                  public sync_fifo_out_if<T, num_write> {
public:
    using sfb = sync_fifo_base<T, max_size, num_read, num_write>;
    using sfb::data;
    using sfb::curr;
    using sfb::size;
    using sfb::read_granted_flag;
    using sfb::read_granted;
    using sfb::read_updated;
    using sfb::write_data;
    using sfb::write_granted_flag;
    using sfb::write_granted;
    using sfb::write_updated;

    sync_fifo(sc_module_name mn) : sfb(mn) { }

    // blocking read
    void read(std::array<T, num_read> &val) override {
        while (size < num_read)
            wait(write_updated);
        for (int i = 0; i < num_read; i++)
            val[i] = data[(curr+i)%max_size];
        read_granted_flag = true;
        read_granted.notify();
    }

    // blocking write
    void write(const std::array<T, num_read> &val) override {
        while (max_size - size < num_write)
            wait(read_updated);
        for (int i = 0; i < num_write; i++)
            write_data[i] = val[i];
        write_granted_flag = true;
        write_granted.notify();
    }

    // direct write
    void direct_write(const std::array<T, num_read> &val) {
        sc_assert(size < max_size);
        data[(curr+size)%max_size] = val;
        size++;
    }
};

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
