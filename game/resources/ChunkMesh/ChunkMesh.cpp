#include "ChunkMesh.hpp"
#include <ChunkNode/ChunkNode.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <array>
#include <vector>

#include "tables.h"

namespace godot {

// ============================================================
//  Вспомогательные константы
// ============================================================

static const Vector3 CUBE_CORNERS[8] = {
    {0,0,0},{1,0,0},{1,1,0},{0,1,0},
    {0,0,1},{1,0,1},{1,1,1},{0,1,1}
};

static const int EDGE_VERTICES[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

// Индексы 6 граней куба: -X, +X, -Y, +Y, -Z, +Z
// Для каждой грани: направление нормали соседа + 4 угловых индекса грани
struct FaceInfo {
    Vector3i dir;           // направление к соседу
    int      corners[4];    // углы куба на этой грани
};

static const FaceInfo FACES[6] = {
    { {-1, 0, 0}, {0, 3, 7, 4} }, // -X
    { { 1, 0, 0}, {1, 2, 6, 5} }, // +X
    { { 0,-1, 0}, {0, 1, 5, 4} }, // -Y
    { { 0, 1, 0}, {2, 3, 7, 6} }, // +Y
    { { 0, 0,-1}, {0, 1, 2, 3} }, // -Z
    { { 0, 0, 1}, {4, 5, 6, 7} }  // +Z
};

// ============================================================
//  Вспомогательные функции
// ============================================================

static Vector3 vertex_interp(float iso, Vector3 p1, Vector3 p2, float v1, float v2) {
    if (Math::abs(v2 - v1) < 1e-4f) return (p1 + p2) * 0.5f;
    float t = (iso - v1) / (v2 - v1);
    return p1 + (p2 - p1) * t;
}

static Vector2 triplanar_uv(const Vector3& pos, const Vector3& normal, float scale = 1.0f) {
    Vector3 blend = Vector3(
        Math::abs(normal.x),
        Math::abs(normal.y),
        Math::abs(normal.z)
    );
    float total = blend.x + blend.y + blend.z + 1e-6f;
    blend /= total;
    Vector2 uv_x = Vector2(pos.z, pos.y) * scale;
    Vector2 uv_y = Vector2(pos.x, pos.z) * scale;
    Vector2 uv_z = Vector2(pos.x, pos.y) * scale;
    return uv_x * blend.x + uv_y * blend.y + uv_z * blend.z;
}

// Вычисляет нормаль треугольника через векторное произведение,
// совпадает с порядком обхода как в оригинальном коде: (b-a) × (c-a).
static Vector3 face_normal(const Vector3& a, const Vector3& b, const Vector3& c) {
    return (b - a).cross(c - a).normalized();
}

static void emit_triangle(Ref<SurfaceTool>& st,
                           const Vector3& a, const Vector3& b, const Vector3& c) {
    Vector3 normal = face_normal(a, b, c);
    if (normal.length_squared() < 1e-8f) return; // вырожденный треугольник

    st->set_normal(normal); st->set_uv(triplanar_uv(a, normal)); st->add_vertex(a);
    st->set_normal(normal); st->set_uv(triplanar_uv(c, normal)); st->add_vertex(c);
    st->set_normal(normal); st->set_uv(triplanar_uv(b, normal)); st->add_vertex(b);
}

// ============================================================
//  ChunkMesh
// ============================================================

// ------------------------------------------------------------
//  Основной марширующий куб
// ------------------------------------------------------------
static void build_marching_cubes(Ref<SurfaceTool>& st,
                                  Ref<BlockSource> p_source,
                                  const Vector3i&  origin,
                                  int              N,
                                  int              step,
                                  float            iso)
{
    for (int ix = 0; ix < N; ++ix) {
        for (int iy = 0; iy < N; ++iy) {
            for (int iz = 0; iz < N; ++iz) {

                Vector3i local_base  = Vector3i(ix * step, iy * step, iz * step);
                Vector3i planet_base = origin + local_base;

                float density[8];
                for (int c = 0; c < 8; ++c) {
                    Vector3i pc = planet_base + Vector3i(
                        (int)CUBE_CORNERS[c].x * step,
                        (int)CUBE_CORNERS[c].y * step,
                        (int)CUBE_CORNERS[c].z * step
                    );
                    density[c] = p_source->get_block(pc).density;
                }

                int cube_idx = 0;
                for (int c = 0; c < 8; ++c)
                    if (density[c] < iso) cube_idx |= (1 << c);

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
                    emit_triangle(st,
                        edge_verts[tris[t    ]],
                        edge_verts[tris[t + 1]],
                        edge_verts[tris[t + 2]]
                    );
                }
            }
        }
    }
}

// ------------------------------------------------------------
//  Правильная юбочка через переинтерполяцию
//
//  Более надёжный подход: на граничной плоскости мы знаем,
//  где у нас вершины. Для каждой вершины на грани строим
//  «стенку» до ближайшей точки на изоповерхности соседнего LOD.
// ------------------------------------------------------------

// Собирает все вершины марширующего куба, лежащие на грани face_index
static std::vector<Vector3> collect_surface_verts_on_face(
    Ref<BlockSource>  p_source,
    const Vector3i&  origin,
    int              N,
    int              step,
    float            iso,
    int              face_index)
{
    std::vector<Vector3> result;
    const FaceInfo& face = FACES[face_index];
    const int* fc = face.corners;

    int axis = face_index / 2;
    int side = face_index % 2;
    int fixed_idx = (side == 0) ? 0 : (N - 1);

    for (int a = 0; a < N; ++a) {
        for (int b = 0; b < N; ++b) {
            int ix, iy, iz;
            if (axis == 0) { ix = fixed_idx; iy = a; iz = b; }
            else if (axis == 1) { ix = a; iy = fixed_idx; iz = b; }
            else               { ix = a; iy = b; iz = fixed_idx; }

            Vector3i local_base  = Vector3i(ix * step, iy * step, iz * step);
            Vector3i planet_base = origin + local_base;

            float density[8];
            for (int c = 0; c < 8; ++c) {
                Vector3i pc = planet_base + Vector3i(
                    (int)CUBE_CORNERS[c].x * step,
                    (int)CUBE_CORNERS[c].y * step,
                    (int)CUBE_CORNERS[c].z * step
                );
                density[c] = p_source->get_block(pc).density;
            }

            int cube_idx = 0;
            for (int c = 0; c < 8; ++c)
                if (density[c] < iso) cube_idx |= (1 << c);

            if (edgeTable[cube_idx] == 0) continue;

            Vector3 local_corners[8];
            for (int c = 0; c < 8; ++c) {
                local_corners[c] = Vector3(
                    local_base.x + CUBE_CORNERS[c].x * step,
                    local_base.y + CUBE_CORNERS[c].y * step,
                    local_base.z + CUBE_CORNERS[c].z * step
                );
            }

            for (int e = 0; e < 12; ++e) {
                if (!(edgeTable[cube_idx] & (1 << e))) continue;
                int v0 = EDGE_VERTICES[e][0];
                int v1 = EDGE_VERTICES[e][1];
                bool v0_on = (v0==fc[0]||v0==fc[1]||v0==fc[2]||v0==fc[3]);
                bool v1_on = (v1==fc[0]||v1==fc[1]||v1==fc[2]||v1==fc[3]);
                if (!v0_on || !v1_on) continue;

                Vector3 p = vertex_interp(iso,
                    local_corners[v0], local_corners[v1],
                    density[v0],       density[v1]);
                result.push_back(p);
            }
        }
    }
    return result;
}

// ------------------------------------------------------------
//  Финальная юбочка: стенка от поверхности вдоль нормали грани
// ------------------------------------------------------------
static void build_lod_skirt(Ref<SurfaceTool>& st,
                             Ref<BlockSource>  p_source,
                             const Vector3i&   origin,
                             int               N,
                             int               my_step,
                             int               neighbor_step,
                             float             iso,
                             int               face_index)
{
    // Глубина юбочки — ровно полшага большего из двух LOD.
    // Это гарантирует перекрытие при любой разнице кратностей.
    float skirt_depth = (float)MAX(my_step, neighbor_step) * 0.5f;

    Vector3 outward = Vector3(
        (float)FACES[face_index].dir.x,
        (float)FACES[face_index].dir.y,
        (float)FACES[face_index].dir.z
    );

    std::vector<Vector3> verts = collect_surface_verts_on_face(
        p_source, origin, N, my_step, iso, face_index);

    // Для каждой пары соседних вершин на грани строим прямоугольник юбочки.
    // Вершины собраны попарно (каждые 2 — одно ребро куба).
    for (size_t i = 0; i + 1 < verts.size(); i += 2) {
        const Vector3& p0 = verts[i];
        const Vector3& p1 = verts[i + 1];

        Vector3 s0 = p0 + outward * skirt_depth;
        Vector3 s1 = p1 + outward * skirt_depth;

        // Квад: p0, p1 (на поверхности) → s0, s1 (смещены наружу)
        emit_triangle(st, p0, p1, s0);
        emit_triangle(st, p1, s1, s0);
    }
}

// ============================================================
//  Публичный метод build
// ============================================================

void ChunkMesh::build(Ref<BlockSource> p_source, ChunkNode* p_chunk) {
    const Vector3i origin    = p_chunk->get_origin();
    const int      voxel_cnt = p_chunk->get_voxel_count();
    const int      step      = p_chunk->get_lod();
    const float    iso       = 0.0f;
    const int      N         = voxel_cnt / step;

    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // 1. Основная геометрия
    build_marching_cubes(st, p_source, origin, N, step, iso);

    // 2. Юбочки на всех 6 гранях, где LOD соседа отличается
    for (int face = 0; face < 6; ++face) {
        int neighbor_lod = p_chunk->get_neighbor_lod(FACES[face].dir);

        // Юбочку строим только если lod'ы разные
        // (при одинаковых — стыковка идеальная без юбки)
        if (neighbor_lod > 0 && neighbor_lod != step) {
            build_lod_skirt(st, p_source, origin, N,
                            step, neighbor_lod,
                            iso, face);
        }
    }

    set_mesh(st->commit());
}

void ChunkMesh::_bind_methods() {}

} // namespace godot