#include "ChunkMesh.hpp"
#include <ChunkNode/ChunkNode.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <array>
#include <vector>

#include "tables.h"

namespace godot {

// ============================================================
//  Вспомогательные функции
// ============================================================

// Вершины единичного куба (локальные смещения от угла куба)
static const Vector3 CUBE_CORNERS[8] = {
    {0,0,0},{1,0,0},{1,1,0},{0,1,0},
    {0,0,1},{1,0,1},{1,1,1},{0,1,1}
};

// Какие два угла соединяет каждое из 12 рёбер куба
static const int EDGE_VERTICES[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

// Линейная интерполяция точки пересечения изоповерхности на ребре
static Vector3 vertex_interp(float iso, Vector3 p1, Vector3 p2, float v1, float v2) {
    if (Math::abs(v2 - v1) < 1e-4f) return (p1 + p2) * 0.5f;
    float t = (iso - v1) / (v2 - v1);
    return p1 + (p2 - p1) * t;
}

// ============================================================
//  ChunkMesh
// ============================================================

ChunkMesh::ChunkMesh() {}
ChunkMesh::~ChunkMesh() {}

static Vector2 triplanar_uv(const Vector3& pos, const Vector3& normal, float scale = 1.0f) {
    Vector3 blend = Vector3(
        Math::abs(normal.x),
        Math::abs(normal.y),
        Math::abs(normal.z)
    );
    // Нормализуем веса чтобы сумма = 1
    float total = blend.x + blend.y + blend.z + 1e-6f;
    blend /= total;

    // UV по каждой плоскости
    Vector2 uv_x = Vector2(pos.z, pos.y) * scale; // проекция YZ
    Vector2 uv_y = Vector2(pos.x, pos.z) * scale; // проекция XZ
    Vector2 uv_z = Vector2(pos.x, pos.y) * scale; // проекция XY

    return uv_x * blend.x + uv_y * blend.y + uv_z * blend.z;
}

Ref<ArrayMesh> ChunkMesh::build(Ref<PlanetData> p_data, ChunkNode* p_chunk) {
    ERR_FAIL_COND_V(!p_data.is_valid(), Ref<ArrayMesh>());
    ERR_FAIL_COND_V(!p_chunk,           Ref<ArrayMesh>());

    const Vector3i origin    = p_chunk->get_origin();
    const int      voxel_cnt = p_chunk->get_voxel_count();
    const int      step      = p_chunk->get_lod();
    const float    iso       = 0.0f;

    const int N = voxel_cnt / step;

    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    for (int ix = 0; ix < N; ++ix) {
        for (int iy = 0; iy < N; ++iy) {
            for (int iz = 0; iz < N; ++iz) {

                // Локальные координаты ячейки (для позиций вершин меша)
                Vector3i local_base = Vector3i(ix * step, iy * step, iz * step);
                // Планетарные координаты (только для чтения плотности)
                Vector3i planet_base = origin + local_base;

                float density[8];
                for (int c = 0; c < 8; ++c) {
                    Vector3i planet_corner = planet_base + Vector3i(
                        (int)CUBE_CORNERS[c].x * step,
                        (int)CUBE_CORNERS[c].y * step,
                        (int)CUBE_CORNERS[c].z * step
                    );
                    density[c] = p_data->get_block(planet_corner, step);
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
                for (int e = 0; e < 12; ++e) {
                    if (edgeTable[cube_idx] & (1 << e)) {
                        int v0 = EDGE_VERTICES[e][0];
                        int v1 = EDGE_VERTICES[e][1];
                        edge_verts[e] = vertex_interp(
                            iso,
                            local_corners[v0], local_corners[v1],
                            density[v0],       density[v1]
                        );
                    }
                }

                const int* tris = triTable[cube_idx];
                for (int t = 0; tris[t] != -1; t += 3) {
                    Vector3 a = edge_verts[tris[t   ]];
                    Vector3 b = edge_verts[tris[t + 1]];
                    Vector3 c = edge_verts[tris[t + 2]];

                    Vector3 normal = (b - a).cross(c - a).normalized();

                    st->set_normal(normal);
                    st->set_uv(triplanar_uv(a, normal));
                    st->add_vertex(a);

                    st->set_normal(normal);
                    st->set_uv(triplanar_uv(c, normal));
                    st->add_vertex(c);

                    st->set_normal(normal);
                    st->set_uv(triplanar_uv(b, normal));
                    st->add_vertex(b);
                }
            }
        }
    }

    return st->commit();
}

void ChunkMesh::_bind_methods() {}

} // namespace godot