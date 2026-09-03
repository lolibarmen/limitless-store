#pragma once

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <Utils/VoxelCache.hpp>
#include <memory>

namespace godot {

struct MeshData {
    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedColorArray   colors;
};

// ЕДИНОЕ ОПРЕДЕЛЕНИЕ: используется и здесь, и в ChunkMeshNode
struct ChunkBuildInput {
    Vector3i chunk_coord;
    float chunk_size;
    float voxel_count;
    int lod_level;
    int stride;
    int step;
    std::shared_ptr<VoxelCache> cache; // Без const, так как мы заполняем его в задаче

    // Вспомогательные методы для удобного чтения из кэша
    inline float get_sdf(Vector3i block_coords) const {
        int x = block_coords.x + 2;
        int y = block_coords.y + 2;
        int z = block_coords.z + 2;
        int n = stride;
        if (x < 0 || x >= n || y < 0 || y >= n || z < 0 || z >= n) return 1.0f;
        return cache->get_sdf(x, y, z);
    }

    inline uint16_t get_material(Vector3i block_coords) const {
        int x = block_coords.x + 2;
        int y = block_coords.y + 2;
        int z = block_coords.z + 2;
        int n = stride;
        if (x < 0 || x >= n || y < 0 || y >= n || z < 0 || z >= n) return 0;
        return cache->get_material(x, y, z);
    }
};

// Функция принимает ссылку на эту единую структуру
MeshData build_neochunk_mesh(const ChunkBuildInput& input);

} // namespace godot