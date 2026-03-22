#include "ChunkMesh.hpp"
#include <ChunkNode/ChunkNode.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <array>
#include <vector>
#include <thread>

#include "tables.h"

using namespace godot;

struct MeshData {
    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedColorArray colors;
};

static const Vector3 CUBE_CORNERS[8] = {
    {0,0,0},{1,0,0},{1,1,0},{0,1,0},
    {0,0,1},{1,0,1},{1,1,1},{0,1,1}
};

static const int EDGE_VERTICES[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

static Vector3 vertex_interp(float iso, Vector3 p1, Vector3 p2, float v1, float v2) {
    if (Math::abs(v2 - v1) < 1e-4f) return (p1 + p2) * 0.5f;
    float t = (iso - v1) / (v2 - v1);
    return p1 + (p2 - p1) * t;
}

static Color color_interp(float iso, Color c1, Color c2, float v1, float v2) {
    if (Math::abs(v2 - v1) < 1e-4f) {
        return Color(
            (c1.r + c2.r) * 0.5f,
            (c1.g + c2.g) * 0.5f,
            (c1.b + c2.b) * 0.5f,
            (c1.a + c2.a) * 0.5f
        );
    }
    float t = (iso - v1) / (v2 - v1);
    return Color(
        c1.r + (c2.r - c1.r) * t,
        c1.g + (c2.g - c1.g) * t,
        c1.b + (c2.b - c1.b) * t,
        c1.a + (c2.a - c1.a) * t
    );
}

static Color material_to_weights(BlockMaterial mat) {
    switch (mat) {
        case BlockMaterial::GRASS:  return Color(1, 0, 0, 0);
        case BlockMaterial::DIRT:   return Color(0, 1, 0, 0);
        case BlockMaterial::STONE:  return Color(0, 0, 1, 0);
        case BlockMaterial::SAND:   return Color(0, 0, 0, 1);
        case BlockMaterial::SNOW:   return Color(0, 0, 1, 0); // временно stone
        case BlockMaterial::GRAVEL: return Color(0, 0, 1, 0);
        default:                    return Color(0, 0, 1, 0);
    }
}

MeshData generate_mesh_data(Ref<BlockSource> p_source, ChunkNode* p_chunk) {
    const Vector3i origin    = p_chunk->get_origin();
    const int      voxel_cnt = p_chunk->get_voxel_count();
    const int      step      = p_chunk->get_lod();
    const float    iso       = 0.0f;
    const int      N         = voxel_cnt / step;

    MeshData result;

    for (int ix = 0; ix < N; ++ix) {
        for (int iy = 0; iy < N; ++iy) {
            for (int iz = 0; iz < N; ++iz) {
                Vector3i local_base  = Vector3i(ix * step, iy * step, iz * step);
                Vector3i planet_base = origin + local_base;

                float density[8];
                Color corner_color[8]; // веса материала каждого угла
                for (int c = 0; c < 8; ++c) {
                    Vector3i planet_corner = planet_base + Vector3i(
                        (int)CUBE_CORNERS[c].x * step,
                        (int)CUBE_CORNERS[c].y * step,
                        (int)CUBE_CORNERS[c].z * step
                    );
                    BlockData bd = p_source->get_block(planet_corner);
                    density[c]      = bd.density;
                    corner_color[c] = material_to_weights(bd.material);
                }

                int cube_idx = 0;
                for (int c = 0; c < 8; ++c) {
                    if (density[c] < iso) cube_idx |= (1 << c);
                }
                if (edgeTable[cube_idx] == 0) continue;

                Vector3 local_corners[8];
                for (int c = 0; c < 8; ++c) {
                    local_corners[c] = Vector3(
                        local_base.x + CUBE_CORNERS[c].x * step,
                        local_base.y + CUBE_CORNERS[c].y * step,
                        local_base.z + CUBE_CORNERS[c].z * step
                    );
                }

                Vector3 edge_verts[12];
                Color   edge_colors[12]; // интерполированные веса на ребре
                for (int e = 0; e < 12; ++e) {
                    if (edgeTable[cube_idx] & (1 << e)) {
                        int v0 = EDGE_VERTICES[e][0];
                        int v1 = EDGE_VERTICES[e][1];
                        edge_verts[e]  = vertex_interp(
                            iso,
                            local_corners[v0], local_corners[v1],
                            density[v0],       density[v1]
                        );
                        edge_colors[e] = color_interp(
                            iso,
                            corner_color[v0], corner_color[v1],
                            density[v0],      density[v1]
                        );
                    }
                }

                const int* tris = triTable[cube_idx];
                for (int t = 0; tris[t] != -1; t += 3) {
                    Vector3 a = edge_verts[tris[t    ]];
                    Vector3 b = edge_verts[tris[t + 1]];
                    Vector3 c = edge_verts[tris[t + 2]];
                    Vector3 normal = (b - a).cross(c - a).normalized();

                    result.vertices.push_back(a);
                    result.vertices.push_back(c);
                    result.vertices.push_back(b);
                    result.normals.push_back(normal);
                    result.normals.push_back(normal);
                    result.normals.push_back(normal);
                    result.colors.push_back(edge_colors[tris[t    ]]);
                    result.colors.push_back(edge_colors[tris[t + 2]]); // c
                    result.colors.push_back(edge_colors[tris[t + 1]]); // b
                }
            }
        }
    }

    return result;
}

void ChunkMesh::build(Ref<BlockSource> p_source, ChunkNode* p_chunk) {
    const uint64_t chunk_id = p_chunk->get_instance_id();
    
    std::thread([this, p_source, chunk_id]() {
        ChunkNode* p_chunk = Object::cast_to<ChunkNode>(ObjectDB::get_instance(chunk_id));
        if (!p_chunk) return;

        const MeshData data = generate_mesh_data(p_source, p_chunk);

        p_chunk = Object::cast_to<ChunkNode>(ObjectDB::get_instance(chunk_id));
        if (!p_chunk) return;

        if (data.vertices.is_empty()) {
            p_chunk->call_deferred("set_mesh", Ref<Mesh>());
            return;
        }

        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays[Mesh::ARRAY_VERTEX] = data.vertices;
        arrays[Mesh::ARRAY_NORMAL] = data.normals;
        arrays[Mesh::ARRAY_COLOR]  = data.colors; // вместо ARRAY_TEX_UV

        Ref<ArrayMesh> mesh;
        mesh.instantiate();
        mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        p_chunk->call_deferred("set_mesh", mesh);
    }).detach();
}