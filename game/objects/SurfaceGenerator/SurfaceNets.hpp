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
    // Целочисленный минимум чанка — узел решётки выборки SDF (чанки выровнены по целой решётке).
    Vector3 chunk_coord;
    float chunk_size;   // ребро чанка в мировых единицах (целое, кратно voxel_count)
    float voxel_count;  // число ячеек решётки вдоль ребра чанка (16)
    float cell_size;    // размер одной воксельной ячейки = chunk_size / voxel_count
    int lod_level;
    int grid_cells;     // целое число ячеек решётки внутри чанка = round(chunk_size / cell_size)
    int stride;         // размер плотного массива кэша с учётом 2-воксельных отступов
    int step;           // шаг выборки SDF в мировых единицах (равен cell_size для ровных чанков)
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
};

// Функция принимает ссылку на эту единую структуру
MeshData build_neochunk_mesh(const ChunkBuildInput& input);

} // namespace godot