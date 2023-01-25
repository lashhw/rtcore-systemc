#ifndef RTCORE_SYSTEMC_ARBITER_HPP
#define RTCORE_SYSTEMC_ARBITER_HPP

#include "../blocking.hpp"

class arbiter : public sc_module,
                public memory_if {
public:
    SC_CTOR(arbiter) {

    }

    void request(to_memory_t &payload) override {
    }
};

#endif //RTCORE_SYSTEMC_ARBITER_HPP
