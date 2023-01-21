#ifndef RTCORE_SYSTEMC_SYNC_FIFO_HPP
#define RTCORE_SYSTEMC_SYNC_FIFO_HPP

template <typename T>
class sync_fifo_in_if : virtual public sc_interface {
public:
    virtual void read(T &) = 0;
};

template <typename T>
class sync_fifo_out_if : virtual public sc_interface {
public:
    virtual void write(const T &) = 0;
};

template <typename T>
using sync_fifo_in = sc_port<sync_fifo_in_if<T>>;

template <typename T>
using sync_fifo_out = sc_port<sync_fifo_out_if<T>>;

template<typename T, size_t max_size, size_t num_read = 1, size_t num_write = 1>
class sync_fifo : public sc_channel,
                  public sync_fifo_in_if<T>,
                  public sync_fifo_out_if<T> {
public:
    SC_CTOR(sync_fifo) {
        curr = 0;
        size = 0;
        num_read_request = 0;
        num_write_request = 0;
    }

    // blocking read
    virtual void read(T &val) {
        size_t id = num_read_request;
        num_read_request++;
        sc_assert(num_read_request <= num_read);
        if (num_read_request == num_read) {
            while (size < num_read)
                wait(write_granted);
            for (int i = 0; i < num_read; i++) {
                read_data[i] = data[curr];
                curr = (curr+1) % max_size;
                size--;
            }
            num_read_request = 0;
            read_granted.notify();
        } else
            wait(read_granted);
        val = read_data[id];
    }

    // blocking write
    virtual void write(const T &val) {
        write_data[num_write_request] = val;
        num_write_request++;
        sc_assert(num_write_request <= num_write);
        if (num_write_request == num_write) {
            wait(1, SC_NS);
            while (max_size - size < num_write)
                wait(read_granted);
            for (int i = 0; i < num_write; i++) {
                data[(curr+size)%max_size] = write_data[i];
                size++;
            }
            num_write_request = 0;
            write_granted.notify();
        } else
            wait(write_granted);
    }

private:
    T data[max_size];
    size_t curr;
    size_t size;

    T read_data[num_read];
    size_t num_read_request;

    T write_data[num_write];
    size_t num_write_request;

    sc_event read_granted;
    sc_event write_granted;
};

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
