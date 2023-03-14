#ifndef RTCORE_SYSTEMC_PAYLOAD_T_HPP
#define RTCORE_SYSTEMC_PAYLOAD_T_HPP

struct ray_t {
    float origin[3];
    float dir[3];
    float t_min;
    float t_max;
};

// TODO: add intersected trig idx
struct result_t {
    int id;
    bool intersected;
    float t;
    float u;
    float v;
};

struct bbox_t {
    float bounds[6];
};

struct node_t {
    bool lp[2];
    int num_trigs;
    uint64_t ptr;
};

struct trig_t {
    float p0[3];
    float p1[3];
    float p2[3];
};

struct ray_and_id_t {
    ray_t ray;
    int id;
};

struct bbox_ctrl_req_t {
    ray_and_id_t ray_and_id;
    bool lp[2];
    uint64_t left_bbox_ptr;
};


struct trv_ctrl_req_t {
    enum { SHADER, BBOX, IST } type;
    union {
        struct {
            ray_and_id_t ray_and_id;
        } shader;
        struct {
            ray_and_id_t ray_and_id;
            uint64_t left_node_ptr;
            bool left_hit;
            bool right_hit;
            bool left_first;
        } bbox;
        struct {
            ray_and_id_t ray_and_id;
            bool intersected;
            float u;
            float v;
        } ist;
    };
};

struct bbox_req_t {
    ray_and_id_t ray_and_id;
    float left_bbox[6];
    float right_bbox[6];
    uint64_t left_node_ptr;
};

struct ist_ctrl_req_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    uint64_t trig_ptr;
    bool intersected;
    float u;
    float v;
};

struct ist_req_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    uint64_t trig_ptr;
    bool intersected;
    float u;
    float v;
    trig_t trig;
};

struct dram_req_t {
    enum { LP_BBOX, HP_BBOX, NODE, TRIG } type;
    uint64_t addr;
};

struct dram_resp_t {
    union {
        bbox_t bbox;
        node_t node;
        trig_t trig;
    };
};

#endif //RTCORE_SYSTEMC_PAYLOAD_T_HPP
