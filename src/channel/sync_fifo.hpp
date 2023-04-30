#ifndef RTCORE_SYSTEMC_SYNC_FIFO_HPP
#define RTCORE_SYSTEMC_SYNC_FIFO_HPP

#include "blocking.hpp"
#include "../misc/utility.hpp"

// read interface for sync_fifo
template <typename T, int num_read>
class sync_fifo_in_if : public blocking_in_if<T> {
public:
    virtual void read(T *) = 0;
};

// write interface for sync_fifo
template <typename T, int num_write>
class sync_fifo_out_if : public blocking_out_if<T> {
public:
    virtual bool writable() = 0;
    using blocking_out_if<T>::write;
    virtual void write(const T *) = 0;
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
        starve_duration = 0;
        stall_duration = 0;

        SC_THREAD(thread_1);
    }

    ~sync_fifo() override {
        std::string mn = name();
        if (size > 0)
            SC_REPORT_WARNING("communication", ("fifo still contains data in " + mn).c_str());
        std::cout << name() << ": STARVE " << starve_duration << " cycles" << std::endl;
        std::cout << name() << ": STALL " << stall_duration << " cycles" << std::endl;
    }

    bool readable() override {
        assert_on_read();
        return size > 0;
    }

    void read(T *val) override {
        sc_time::value_type start_time = curr_cycle();
        while (size < num_read) {
            wait(write_updated);
            delay(1);
        }
        advance_to_read();
        starve_duration += curr_cycle() - start_time;
        for (int i = 0; i < num_read; i++)
            val[i] = data[(curr+i)%max_size];
        read_granted_flag = true;
        read_granted.notify();
    }

    T read() override {
        if (num_read != 1)
            SC_REPORT_FATAL("other", "read() should only be used when num_read == 1");
        T tmp;
        read(&tmp);
        return tmp;
    }

    bool writable() override {
        assert_on_write();
        return max_size - size >= num_write;
    }

    void write(const T *val) override {
        sc_time::value_type start_time = curr_cycle();
        while (max_size - size < num_write) {
            wait(read_updated);
            delay(1);
        }
        advance_to_write();
        stall_duration += curr_cycle() - start_time;
        for (int i = 0; i < num_write; i++)
            data[(curr+size+i)%max_size] = val[i];
        write_granted_flag = true;
        write_granted.notify();
    }

    void write(const T &val) override {
        if (num_write != 1)
            SC_REPORT_FATAL("other", "write(const T &) should only be used when num_write == 1");
        write(&val);
    }

    void direct_write(const T *val) {
        if (max_size - size < num_write)
            SC_REPORT_FATAL("other", "fifo does not have enough space");
        for (int i = 0; i < num_write; i++) {
            data[(curr+size)%max_size] = val[i];
            size++;
        }
    }

    void direct_write(const T &val) {
        if (num_write != 1)
            SC_REPORT_FATAL("other", "direct_write(const T &) should only be used when num_write == 1");
        direct_write(&val);
    }

private:
    void thread_1() {
        while (true) {
            wait(read_granted | write_granted);
            advance_to_update();

            // read
            if (read_granted_flag) {
                curr = (curr + num_read) % max_size;
                size -= num_read;
                read_granted_flag = false;
                read_updated.notify();
            }

            // write
            if (write_granted_flag) {
                size += num_write;
                write_granted_flag = false;
                write_updated.notify();
            }
        }
    }

    T data[max_size];
    int curr;
    int size;

    bool read_granted_flag;
    sc_event read_granted;
    sc_event read_updated;

    bool write_granted_flag;
    sc_event write_granted;
    sc_event write_updated;

    sc_time::value_type starve_duration;
    sc_time::value_type stall_duration;
};

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
