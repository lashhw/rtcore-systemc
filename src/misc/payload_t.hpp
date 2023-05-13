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
    bool left_lp;
    bool right_lp;
    int num_trigs;
    uint64_t addr;
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
    bool left_lp;
    bool right_lp;
    uint64_t left_bbox_addr;
};


struct trv_ctrl_req_t {
    enum { SHADER, BBOX, IST } type;
    union {
        struct {
            ray_and_id_t ray_and_id;
        } shader;
        struct {
            ray_and_id_t ray_and_id;
            node_t left_node;
            node_t right_node;
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
    bbox_t left_bbox;
    bbox_t right_bbox;
    node_t left_node;
    node_t right_node;
};

struct ist_ctrl_req_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    uint64_t trig_addr;
    bool intersected;
    float u;
    float v;
};

struct ist_req_t {
    ray_and_id_t ray_and_id;
    int num_trigs;
    uint64_t trig_addr;
    bool intersected;
    float u;
    float v;
    trig_t trig;
};

struct dram_req_t {
    uint64_t addr;
    int num_bytes;
};

enum class dram_type_t {
    BBOX, NODE, TRIG
};

struct dram_data_t {
    union {
        bbox_t bbox;
        node_t node;
        trig_t trig;
    };
};

template <typename additional_t>
struct l1c_req_t {
    uint64_t addr;
    int num_bytes;
    additional_t additional;
};

template <typename additional_t>
struct l1c_resp_t {
    uint64_t addr;
    additional_t additional;
};

struct bbox_additional_t {
    ray_and_id_t ray_and_id;
    bool left_lp;
    bool right_lp;
};

struct ist_additional_t {
    ray_and_id_t ray_and_id;
};

typedef l1c_req_t<bbox_additional_t> bbox_l1c_req_t;
typedef l1c_resp_t<bbox_additional_t> bbox_l1c_resp_t;
typedef l1c_req_t<ist_additional_t> ist_l1c_req_t;
typedef l1c_resp_t<ist_additional_t> ist_l1c_resp_t;

#endif //RTCORE_SYSTEMC_PAYLOAD_T_HPP
