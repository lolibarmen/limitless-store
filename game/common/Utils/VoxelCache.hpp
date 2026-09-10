#pragma once

#include <godot_cpp/variant/vector3i.hpp>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace godot {

class VoxelCache {
public:
    int stride;
    int step;
    Vector3i chunk_coord;
    
    // Плотные массивы данных
    std::vector<float> sdf_data;
    // std::vector<uint16_t> material_data;

    VoxelCache(int p_stride, int p_step, const Vector3i& p_coord) 
        : stride(p_stride), step(p_step), chunk_coord(p_coord) 
    {
        size_t total = stride * stride * stride;
        sdf_data.assign(total, 1e10f);
        // material_data.assign(total, 0);
    }

    inline float get_sdf(int x, int y, int z) const {
        int idx = (x * stride + y) * stride + z;
        return sdf_data[idx];
    }

    // inline uint16_t get_material(int x, int y, int z) const {
    //     int idx = (x * stride + y) * stride + z;
    //     return material_data[idx];
    // }

    inline void set(int x, int y, int z, float sdf, uint16_t mat) {
        int idx = (x * stride + y) * stride + z;
        sdf_data[idx] = sdf;
        // material_data[idx] = mat;
    }
};

} // namespace godot