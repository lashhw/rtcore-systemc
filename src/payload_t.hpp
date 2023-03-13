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

struct trig_t {
    float p0[3];
    float p1[3];
    float p2[3];
};

struct node_t {
    bool lp[2];
    int num_trigs;
    uint64_t ptr;
    float bbox[6];
};

struct ray_and_id_t {
    ray_t ray;
    int id;
};

struct bbox_ctrl_req_t {
    ray_and_id_t ray_and_id;
    int left_node_idx;
};

struct bbox_result_t {
    ray_and_id_t ray_and_id;
    int left_node_idx;
    bool left_hit;
    bool right_hit;
    bool left_first;
};

struct ist_result_t {
    ray_and_id_t ray_and_id;
    bool intersected;
    float u;
    float v;
};

struct trv_ctrl_req_t {
    enum { SHADER, BBOX, IST } type;
    union {
        ray_and_id_t shader;
        bbox_result_t bbox;
        ist_result_t ist;
    };
};

struct bbox_req_t {
    ray_and_id_t ray_and_id;
    int left_node_idx;
    float left_bbox[6];
    float right_bbox[6];
};

struct ist_ctrl_req_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    int first_trig_idx;
    bool intersected;
    float u;
    float v;
};

struct ist_req_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    int first_trig_idx;
    bool intersected;
    float u;
    float v;
    trig_t trig;
};

struct mem_req_t {
    enum { BBOX, NODE, TRIG_IDX, TRIG } type;
    int idx;
};

struct mem_resp_t {
    union {
        struct {
            bool low_precision;
            float bounds[6];
        } bbox;
        int node[2];
        int trig_idx;
        trig_t trig;
    };
};

#endif //RTCORE_SYSTEMC_PAYLOAD_T_HPP
