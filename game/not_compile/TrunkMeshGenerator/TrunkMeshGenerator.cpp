#include "TrunkMeshGenerator.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/core/math.hpp>
#include <algorithm>
#include <vector>

using namespace godot;

void TrunkMeshGenerator::_bind_methods() { }

// Детерминированный hash -> float в [-1, 1]
static float _hash_f(uint64_t x) {
    x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return ((float)(x & 0xFFFFFFu) / 8388608.0f) - 1.0f;
}

Ref<ArrayMesh> TrunkMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<ArrayMesh> mesh;
    mesh.instantiate();

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return mesh;
    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return mesh;
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return mesh;

    auto* world = SemanticWorld::get_singleton();
    Ref<SemanticShape> shape = world->get_shape(shape_id);
    if (!shape.is_valid()) return mesh;

    AABB sa = shape->get_aabb();
    AABB ca = chunk->get_aabb();
    AABB it = sa.intersection(ca);
    const float EPS = 0.001f;
    if (it.size.y <= EPS || it.size.x <= EPS || it.size.z <= EPS) return mesh;

    const float H = MAXF(sa.size.y, EPS);
    const float bottom = sa.position.y;
    const float top = sa.position.y + sa.size.y;
    const Vector2 axis(sa.position.x + sa.size.x * 0.5f, sa.position.z + sa.size.z * 0.5f);
    const float r_bot = MINF(sa.size.x, sa.size.z) * 0.5f;
    const float r_top = r_bot * taper;

    // Контрольные смещения изгиба (4 точки: t = 0, 1/3, 2/3, 1)
    const int M = 3;
    Vector2 ctrl[M + 1];
    for (int j = 0; j <= M; ++j) {
        ctrl[j].x = _hash_f(shape_id * 7u  + (uint64_t)j * 131u) * bend_amount;
        ctrl[j].y = _hash_f(shape_id * 13u + (uint64_t)j * 97u + 1u) * bend_amount;
    }
    auto bend = [&](float t) -> Vector2 {
        float s = CLAMPF(t, 0.0f, 1.0f) * (float)M;
        int i0 = (int)s; if (i0 >= M) i0 = M - 1;
        float f = s - (float)i0;
        return ctrl[i0] + (ctrl[i0 + 1] - ctrl[i0]) * f;
    };

    // Глобальные y-уровни сетки, попавшие в чанк + границы чанка (для бесшовности)
    std::vector<float> ys;
    const float step = H / (float)height_segments;
    for (int k = 0; k <= height_segments; ++k) {
        float y = bottom + (float)k * step;
        if (y > it.position.y + EPS && y < it.position.y + it.size.y - EPS) ys.push_back(y);
    }
    ys.push_back(it.position.y);
    ys.push_back(it.position.y + it.size.y);
    std::sort(ys.begin(), ys.end());
    // убрать почти-дубли
    std::vector<float> yl;
    for (float y : ys) if (yl.empty() || y - yl.back() > EPS) yl.push_back(y);

    const Vector3 cc = ca.get_center();
    const int R = radial_segments;

    PackedVector3Array vertices, normals;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    // Строим кольца; запоминаем базовый индекс вершин каждого кольца
    std::vector<int> ring_base;
    ring_base.reserve(yl.size());
    for (float y : yl) {
        float t = (y - bottom) / H;
        float r = r_bot + (r_top - r_bot) * t;
        Vector2 off = bend(t);
        Vector3 cw(axis.x + off.x, y, axis.y + off.y); // world центр кольца
        Vector3 cl = cw - cc;                          // local
        int base = (int)vertices.size();
        ring_base.push_back(base);
        for (int j = 0; j < R; ++j) {
            float a = (float)j / (float)R * (float)Math_PI * 2.0f;
            float cs = std::cos(a), sn = std::sin(a);
            vertices.push_back(cl + Vector3(cs * r, 0.0f, sn * r));
            normals.push_back(Vector3(cs, 0.0f, sn));
            uvs.push_back(Vector2((float)j / (float)R, t));
        }
    }

    // Боковая поверхность между соседними кольцами
    for (size_t i = 0; i + 1 < ring_base.size(); ++i) {
        int a = ring_base[i], b = ring_base[i + 1];
        for (int j = 0; j < R; ++j) {
            int jn = (j + 1) % R;
            indices.push_back(a + j);     indices.push_back(b + j);     indices.push_back(a + jn);
            indices.push_back(a + jn);    indices.push_back(b + j);     indices.push_back(b + jn);
        }
    }

    // Крышка сверху — только если ствол реально заканчивается в этом чанке
    if (!yl.empty() && (yl.back() - top) < EPS) {
        int base = (int)vertices.size();
        float t = 1.0f;
        Vector2 off = bend(t);
        Vector3 cw(axis.x + off.x, top, axis.y + off.y);
        Vector3 cl = cw - cc;
        vertices.push_back(cl); normals.push_back(Vector3(0, 1, 0)); uvs.push_back(Vector2(0.5f, 1.0f));
        int rb = ring_base.back();
        for (int j = 0; j < R; ++j) {
            int jn = (j + 1) % R;
            indices.push_back(base); indices.push_back(rb + j); indices.push_back(rb + jn);
        }
    }

    if (vertices.is_empty()) return mesh;

    Array arr;
    arr.resize(Mesh::ARRAY_MAX);
    arr[Mesh::ARRAY_VERTEX] = vertices;
    arr[Mesh::ARRAY_NORMAL] = normals;
    arr[Mesh::ARRAY_TEX_UV] = uvs;
    arr[Mesh::ARRAY_INDEX] = indices;
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);
    return mesh;
}