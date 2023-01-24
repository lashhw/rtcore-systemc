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

template<typename T, int max_size, int num_read, int num_write>
class sync_fifo_base : virtual public sync_fifo_base_in_if {
public:
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
            bool r = num_read_request == num_read && size >= num_read;
            bool w = num_write_request == num_write && max_size - size >= num_write;

            // read
            if (r) {
                curr = (curr + num_read) % max_size;
                size -= num_read;
                num_read_request = 0;
                read_updated.notify(0.5, SC_NS);
            }

            // write
            if (w) {
                for (int i = 0; i < num_write; i++) {
                    data[(curr+size)%max_size] = write_data[i];
                    size++;
                }
                num_write_request = 0;
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

    T read_data[num_read];
    int num_read_request;

    T write_data[num_write];
    int num_write_request;

    sc_event read_granted;
    sc_event read_updated;
    sc_event write_granted;
    sc_event write_updated;
};

template<typename T, int max_size, int num_read = 1, int num_write = 1>
class sync_fifo : public sc_channel,
                  public sync_fifo_base<T, max_size, num_read, num_write>,
                  public sync_fifo_in_if<T, num_read>,
                  public sync_fifo_out_if<T, num_write> {
public:
    using sync_fifo_base<T, max_size, num_read, num_write>::data;
    using sync_fifo_base<T, max_size, num_read, num_write>::curr;
    using sync_fifo_base<T, max_size, num_read, num_write>::size;
    using sync_fifo_base<T, max_size, num_read, num_write>::read_data;
    using sync_fifo_base<T, max_size, num_read, num_write>::num_read_request;
    using sync_fifo_base<T, max_size, num_read, num_write>::write_data;
    using sync_fifo_base<T, max_size, num_read, num_write>::num_write_request;
    using sync_fifo_base<T, max_size, num_read, num_write>::read_granted;
    using sync_fifo_base<T, max_size, num_read, num_write>::read_updated;
    using sync_fifo_base<T, max_size, num_read, num_write>::write_granted;
    using sync_fifo_base<T, max_size, num_read, num_write>::write_updated;
    using sync_fifo_base<T, max_size, num_read, num_write>::main_thread;

    SC_CTOR(sync_fifo) {
        curr = 0;
        size = 0;
        num_read_request = 0;
        num_write_request = 0;
        SC_THREAD(main_thread);
    }

    // blocking read
    void read(T &val) override {
        int id = num_read_request;
        num_read_request++;
        sc_assert(num_read_request <= num_read);
        if (num_read_request == num_read) {
            while (size < num_read)
                wait(write_updated);
            for (int i = 0; i < num_read; i++)
                read_data[i] = data[(curr+i)%max_size];
            read_granted.notify();
        } else
            wait(read_granted);
        val = read_data[id];
    }

    // blocking write
    void write(const T &val) override {
        write_data[num_write_request] = val;
        num_write_request++;
        sc_assert(num_write_request <= num_write);
        if (num_write_request == num_write) {
            while (max_size - size < num_write)
                wait(read_updated);
            write_granted.notify();
        } else
            wait(write_granted);
    }

    // direct write
    void direct_write(const T &val) {
        sc_assert(size < max_size);
        data[(curr+size)%max_size] = val;
        size++;
    }
};

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
