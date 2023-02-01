#ifndef RTCORE_SYSTEMC_SYNC_FIFO_HPP
#define RTCORE_SYSTEMC_SYNC_FIFO_HPP

#include "../misc.hpp"

// input interface for sync_fifo
template <typename T, int num_read>
class sync_fifo_in_if : virtual public sc_interface {
public:
    virtual const sc_event &data_written_event() const = 0;
    virtual const int &num_elements() const = 0;
    virtual void read(T *) = 0;
    virtual void read(T &) = 0;
    virtual T read() = 0;
};

// output interface for sync_fifo
template <typename T, int num_write>
class sync_fifo_out_if : virtual public sc_interface {
public:
    virtual void write(const T *) = 0;
    virtual void write(const T &) = 0;
};

// alias for sc_port<sync_fifo_in_if<T, num_read>>
template <typename T, int num_read = 1>
using sync_fifo_in = sc_port<sync_fifo_in_if<T, num_read>>;

// alias for sc_port<sync_fifo_out_if<T, num_write>>
template <typename T, int num_write = 1>
using sync_fifo_out = sc_port<sync_fifo_out_if<T, num_write>>;

// the channel that implements sync_fifo_in_if and sync_fifo_out_if
template<typename T, int max_size, int num_read = 1, int num_write = 1>
class sync_fifo : public sc_channel,
                  public sync_fifo_in_if<T, num_read>,
                  public sync_fifo_out_if<T, num_write> {
public:
    SC_CTOR(sync_fifo) {
        curr = 0;
        size = 0;
        read_granted_flag = false;
        write_granted_flag = false;
        SC_THREAD(main_thread);
    }

    const sc_event &data_written_event() const override {
        return write_updated;
    }

    const int &num_elements() const override {
        return size;
    }

    // blocking read
    void read(T *val) override {
        while (size < num_read)
            wait(write_updated);
        for (int i = 0; i < num_read; i++)
            val[i] = data[(curr+i)%max_size];
        read_granted_flag = true;
        read_granted.notify();
    }

    // this method should only be used when num_read = 1
    void read(T &val) override {
        sc_assert(num_read == 1);
        read(&val);
    }

    T read() override {
        T tmp;
        read(tmp);
        return tmp;
    }

    // blocking write
    void write(const T *val) override {
        while (max_size - size < num_write)
            wait(read_updated);
        for (int i = 0; i < num_write; i++)
            write_data[i] = val[i];
        write_granted_flag = true;
        write_granted.notify();
    }

    // this method should only be used when num_write = 1
    void write(const T &val) override {
        sc_assert(num_write == 1);
        write(&val);
    }

    // direct write
    void direct_write(const T *val) {
        sc_assert(max_size - size >= num_write);
        for (int i = 0; i < num_write; i++) {
            data[(curr+size)%max_size] = val[i];
            size++;
        }
    }

    // this method should only be used when num_write = 1
    void direct_write(const T &val) {
        sc_assert(num_write == 1);
        direct_write(&val);
    }

private:
    void main_thread() {
        while (true) {
            wait(read_granted | write_granted);
            wait(half_cycle);

            // read
            if (read_granted_flag) {
                curr = (curr + num_read) % max_size;
                size -= num_read;
                read_granted_flag = false;
                read_updated.notify(half_cycle);
            }

            // write
            if (write_granted_flag) {
                for (int i = 0; i < num_write; i++) {
                    data[(curr+size)%max_size] = write_data[i];
                    size++;
                }
                write_granted_flag = false;
                write_updated.notify(half_cycle);
            }

#ifndef NDEBUG
            // print
            std::cout << "sync_fifo @ " << sc_time_stamp() << ": ";
            for (int i = 0; i < size; i++)
                std::cout << data[(curr+i)%max_size] << " ";
            std::cout << std::endl;
#endif
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

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
