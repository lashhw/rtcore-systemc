#ifndef RTCORE_SYSTEMC_PAYLOAD_T_HPP
#define RTCORE_SYSTEMC_PAYLOAD_T_HPP

struct ray_and_id_t {
    bvh::Ray<float> ray;
    int id;
};

struct to_trv_ctrl_t {
    enum { SHADER, LP, HP, IST } type;
    union {
        ray_and_id_t ray_and_id;
    } payload;
};

struct to_memory_t {
    enum { BBOX, NODE, TRIG_IDX, TRIG } type;
    int idx;
};

struct from_memory_t {
    union {
        float *bbox;
        bvh::Bvh<float>::IndexType *node;
        size_t *trig_idx;
        bvh::Triangle<float> *trig;
    } response[2];
};


#endif //RTCORE_SYSTEMC_PAYLOAD_T_HPP
