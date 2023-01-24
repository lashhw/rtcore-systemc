#ifndef RTCORE_SYSTEMC_PAYLOAD_T_HPP
#define RTCORE_SYSTEMC_PAYLOAD_T_HPP

struct shader_to_trv_ctrl_t {
    bvh::Ray<float> ray;
    int id;
};

struct trv_ctrl_to_shader_t {
    bvh::Triangle<float>::Intersection ist_result;
    int id;
};

struct trv_ctrl_to_bbox_ctrl_t {

};

struct bbox_ctrl_to_lp_t {

};

struct bbox_ctrl_to_hp_t {

};

struct lp_to_trv_ctrl_t {

};

struct hp_to_trv_ctrl_t {

};

struct trv_ctrl_to_ist_ctrl_t {

};

struct ist_ctrl_to_ist_t {

};

struct ist_to_trv_ctrl_t {

};

struct to_memory_t {
    enum { BBOX, NODE, TRIG_IDX, TRIG } type;
    int idx;
    union {
        float *bbox;
        bvh::Bvh<float>::IndexType *node;
        size_t *trig_idx;
        bvh::Triangle<float> *trig;
    } response[2];
};


#endif //RTCORE_SYSTEMC_PAYLOAD_T_HPP
