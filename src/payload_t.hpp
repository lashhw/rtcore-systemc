#ifndef RTCORE_SYSTEMC_PAYLOAD_T_HPP
#define RTCORE_SYSTEMC_PAYLOAD_T_HPP

struct ray_t {
    float origin[3];
    float dir[3];
    float t_min;
    float t_max;
};

struct result_t {
    int id;
};

struct trig_t {
    float p0[3];
    float e1[3];
    float e2[3];
};

struct ray_and_id_t {
    ray_t ray;
    int id;
};

struct to_bbox_ctrl_t {
    ray_and_id_t ray_and_id;
    int node_idx;
};

struct from_bbox_t {
    ray_and_id_t ray_and_id;
    int left_node_idx;
    bool hit[2];
    bool left_first;
};

struct to_trv_ctrl_t {
    enum { SHADER, BBOX, IST } type;
    union {
        ray_and_id_t ray_and_id;
        from_bbox_t from_bbox;
    };
};

struct to_bbox_t {

};

struct to_ist_ctrl_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    int first_trig_idx;
};

struct to_memory_t {
    enum { BBOX, NODE, TRIG_IDX, TRIG } type;
    int idx;
};

struct to_thread_2_t {
    ray_and_id_t ray_and_id;
    to_memory_t to_memory;
};

struct from_memory_t {
    union {
        float bbox[6];
        int node[2];
        int trig_idx;
        trig_t trig;
    };
};

#endif //RTCORE_SYSTEMC_PAYLOAD_T_HPP
