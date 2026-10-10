#include "CubeMeshGenerator.hpp"
#include <ChunkOctree/ChunkOctree.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

CubeMeshGenerator::CubeMeshGenerator() = default;
CubeMeshGenerator::~CubeMeshGenerator() = default;

void CubeMeshGenerator::_bind_methods() { }

Ref<ArrayMesh> CubeMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<ArrayMesh> mesh;
    mesh.instantiate();

    Chunk* chunk = ChunkOctree::get_singleton()->find(chunk_id);
    if (!chunk) return mesh;

    AABB chunk_aabb = chunk->get_aabb();
    
    Vector3 c_min = pos;
    Vector3 c_max = pos + size;
    
    Vector3 ch_min = chunk_aabb.position;
    Vector3 ch_max = chunk_aabb.position + chunk_aabb.size;

    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    auto add_quad = [&](Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3, Vector3 normal) {
        int base = vertices.size();
        vertices.push_back(v0); vertices.push_back(v1);
        vertices.push_back(v2); vertices.push_back(v3);
        
        for (int i = 0; i < 4; i++) normals.push_back(normal);
        
        uvs.push_back(Vector2(0, 1)); uvs.push_back(Vector2(0, 0));
        uvs.push_back(Vector2(1, 0)); uvs.push_back(Vector2(1, 1));
        
        indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base); indices.push_back(base + 3); indices.push_back(base + 2);
    };

    float x0, x1, y0, y1, z0, z1;

    if (c_max.x >= ch_min.x && c_max.x <= ch_max.x) {
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (y0 < y1 && z0 < z1) 
            add_quad(Vector3(c_max.x, y0, z0), Vector3(c_max.x, y1, z0), Vector3(c_max.x, y1, z1), Vector3(c_max.x, y0, z1), Vector3(1, 0, 0));
    }

    if (c_min.x >= ch_min.x && c_min.x <= ch_max.x) {
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (y0 < y1 && z0 < z1) 
            add_quad(Vector3(c_min.x, y0, z1), Vector3(c_min.x, y1, z1), Vector3(c_min.x, y1, z0), Vector3(c_min.x, y0, z0), Vector3(-1, 0, 0));
    }

    if (c_max.y >= ch_min.y && c_max.y <= ch_max.y) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (x0 < x1 && z0 < z1) 
            add_quad(Vector3(x0, c_max.y, z1), Vector3(x1, c_max.y, z1), Vector3(x1, c_max.y, z0), Vector3(x0, c_max.y, z0), Vector3(0, 1, 0));
    }

    if (c_min.y >= ch_min.y && c_min.y <= ch_max.y) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (x0 < x1 && z0 < z1) 
            add_quad(Vector3(x0, c_min.y, z0), Vector3(x1, c_min.y, z0), Vector3(x1, c_min.y, z1), Vector3(x0, c_min.y, z1), Vector3(0, -1, 0));
    }

    if (c_max.z >= ch_min.z && c_max.z <= ch_max.z) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        if (x0 < x1 && y0 < y1) 
            add_quad(Vector3(x0, y0, c_max.z), Vector3(x1, y0, c_max.z), Vector3(x1, y1, c_max.z), Vector3(x0, y1, c_max.z), Vector3(0, 0, 1));
    }

    if (c_min.z >= ch_min.z && c_min.z <= ch_max.z) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        if (x0 < x1 && y0 < y1) 
            add_quad(Vector3(x1, y0, c_min.z), Vector3(x0, y0, c_min.z), Vector3(x0, y1, c_min.z), Vector3(x1, y1, c_min.z), Vector3(0, 0, -1));
    }

    if (vertices.is_empty()) return mesh;

    Array mesh_array;
    mesh_array.resize(Mesh::ARRAY_MAX);
    mesh_array[Mesh::ARRAY_VERTEX] = vertices;
    mesh_array[Mesh::ARRAY_NORMAL] = normals;
    mesh_array[Mesh::ARRAY_TEX_UV] = uvs;
    mesh_array[Mesh::ARRAY_INDEX] = indices;

    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);

    return mesh;
}