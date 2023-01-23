#ifndef RTCORE_SYSTEMC_MEMORY_IF_HPP
#define RTCORE_SYSTEMC_MEMORY_IF_HPP

struct mem_payload_t {
    enum { BBOX, NODE, TRIG_IDX, TRIG } type;
    int idx;
    union {
        float *bbox;
        bvh::Bvh<float>::IndexType *node;
        size_t *trig_idx;
        bvh::Triangle<float> *trig;
    } response[2];
};

class memory_if : virtual public sc_interface {
public:
    virtual void request(mem_payload_t &payload) = 0;
};

#endif //RTCORE_SYSTEMC_MEMORY_IF_HPP
