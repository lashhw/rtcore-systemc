#ifndef RTCORE_SYSTEMC_SYNC_FIFO_HPP
#define RTCORE_SYSTEMC_SYNC_FIFO_HPP

#include "../blocking.hpp"
#include "../params.hpp"

// input interface for sync_fifo
template <typename T, int num_read>
class sync_fifo_in_if : public blocking_in_if<T> {
public:
    virtual const int &num_elements() const = 0;
    virtual void read(T *) = 0;
};

// output interface for sync_fifo
template <typename T, int num_write>
class sync_fifo_out_if : public blocking_out_if<T> {
public:
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
    enum state_t {
        UTILIZE, STARVE, STALL
    };

    SC_CTOR(sync_fifo) {
        curr = 0;
        size = 0;
        read_granted_flag = false;
        write_granted_flag = false;
        last_state = UTILIZE;
        last_state_time = sc_time();
        utilize_duration = sc_time();
        starve_duration = sc_time();
        stall_duration = sc_time();
        SC_THREAD(main_thread);
    }

    ~sync_fifo() override {
        if (size > 0)
            std::cerr << name() << " still contains data" << std::endl;
        update_state(UTILIZE);
        std::cout << name() << ": UTILIZE " << utilize_duration << std::endl;
        std::cout << name() << ": STARVE " << starve_duration << std::endl;
        std::cout << name() << ": STALL " << stall_duration << std::endl;
    }

    const sc_event &data_written_event() const override {
        return write_updated;
    }

    bool data_written() const override {
        return num_elements() > 0;
    }

    const int &num_elements() const override {
        return size;
    }

    // blocking read
    void read(T *val) override {
        if (size < num_read) {
            update_state(STARVE);
            while (size < num_read)
                wait(write_updated);
            update_state(UTILIZE);
        }
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
        if (max_size - size < num_write) {
            update_state(STALL);
            while (max_size - size < num_write)
                wait(read_updated);
            update_state(UTILIZE);
        }
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

    void update_state(state_t new_state) {
        switch (last_state) {
            case UTILIZE:
                utilize_duration += sc_time_stamp() - last_state_time;
                break;
            case STARVE:
                starve_duration += sc_time_stamp() - last_state_time;
                break;
            case STALL:
                stall_duration += sc_time_stamp() - last_state_time;
                break;
        }
        last_state = new_state;
        last_state_time = sc_time_stamp();
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

    state_t last_state;
    sc_time last_state_time;
    sc_time utilize_duration;
    sc_time starve_duration;
    sc_time stall_duration;
};

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
