#pragma once


#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <array>
#include <optional>

#include <GameStructs/Blocks.hpp>
#include <BlockLODSource/BlockLODSource.hpp>

using namespace godot;

struct MeshData {
    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedColorArray   colors;   // R = material/255, G = density, B = 0, A = 0
};

struct ChunkBuildInput {
    Vector3i            chunk_coord;         // мировой угол чанка в блоках(не центр!)
    float               chunk_size;          // размер стороны блока
    float               voxel_count;         // количество блоков в чанке
    int                 lod_level;           // глубина в дереве (0 = корень)
    std::vector<BlockData> blocks;

    BlockData get_block(Vector3i block_coords) const;
};

MeshData build_neochunk_mesh(const ChunkBuildInput& input);