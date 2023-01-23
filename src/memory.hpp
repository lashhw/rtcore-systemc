#ifndef RTCORE_SYSTEMC_MEMORY_HPP
#define RTCORE_SYSTEMC_MEMORY_HPP

#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include "third_party/happly/happly.h"

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

class memory : public sc_channel,
               public memory_if {
public:
    SC_HAS_PROCESS(memory);
    memory(sc_module_name mn, const char *model_ply_path) : sc_module(mn) {
        happly::PLYData ply_data(model_ply_path);
        std::vector<std::array<double, 3>> v_pos = ply_data.getVertexPositions();
        std::vector<std::vector<size_t>> f_idx = ply_data.getFaceIndices<size_t>();

        for (auto &face : f_idx) {
            triangles.emplace_back(bvh::Vector3<float>(v_pos[face[0]][0], v_pos[face[0]][1], v_pos[face[0]][2]),
                                   bvh::Vector3<float>(v_pos[face[1]][0], v_pos[face[1]][1], v_pos[face[1]][2]),
                                   bvh::Vector3<float>(v_pos[face[2]][0], v_pos[face[2]][1], v_pos[face[2]][2]));
        }

        auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
        auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), triangles.size());
        std::cout << "global bounding box: ("
                  << global_bbox.min[0] << ", " << global_bbox.min[1] << ", " << global_bbox.min[2] << "), ("
                  << global_bbox.max[0] << ", " << global_bbox.max[1] << ", " << global_bbox.max[2] << ")" << std::endl;

        bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
        builder.build(global_bbox, bboxes.get(), centers.get(), triangles.size());
    }

    void request(mem_payload_t &payload) override {
        switch (payload.type) {
            case mem_payload_t::BBOX:
                payload.response[0].bbox = bvh.nodes[payload.idx].bounds;
                break;
            case mem_payload_t::NODE:
                payload.response[0].node = &bvh.nodes[payload.idx].primitive_count;
                payload.response[1].node = &bvh.nodes[payload.idx].first_child_or_primitive;
                break;
            case mem_payload_t::TRIG_IDX:
                payload.response[0].trig_idx = &bvh.primitive_indices[payload.idx];
                break;
            case mem_payload_t::TRIG:
                payload.response[0].trig = &triangles[payload.idx];
                break;
        }
    }

private:
    std::vector<bvh::Triangle<float>> triangles;
    bvh::Bvh<float> bvh;
};

#endif //RTCORE_SYSTEMC_MEMORY_HPP
