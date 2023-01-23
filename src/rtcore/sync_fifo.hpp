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

template<typename T, int max_size, int num_read = 1, int num_write = 1>
class sync_fifo : public sc_channel,
                  public sync_fifo_in_if<T>,
                  public sync_fifo_out_if<T> {
public:
    SC_HAS_PROCESS(sync_fifo);
    sync_fifo(sc_module_name mn) {
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

private:
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
            std::cout << name() << " @ " << sc_time_stamp() << ": ";
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

#endif //RTCORE_SYSTEMC_SYNC_FIFO_HPP
