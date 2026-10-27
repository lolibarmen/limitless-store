#include "MeshGenerator.hpp"
#include "WorldMesh/WorldMesh.hpp"
#include "ShapeSpatialManager/ShapeSpatialManager.hpp"
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/variant/array.hpp>
#include <algorithm>

using namespace godot;

void MeshGenerator::_bind_methods() {}

void MeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    // Базовая реализация пуста; дочерний переопределяет и заканчивает submit().
}

void MeshGenerator::MeshBuilder::emit(const Vertex& a, const Vertex& b, const Vertex& c) {
    bool crossing = false;
    if (filter) {
        const Vector3 lo = box.position, hi = box.position + box.size;
        const real_t mnx = std::min({a.pos.x, b.pos.x, c.pos.x}), mxx = std::max({a.pos.x, b.pos.x, c.pos.x});
        const real_t mny = std::min({a.pos.y, b.pos.y, c.pos.y}), mxy = std::max({a.pos.y, b.pos.y, c.pos.y});
        const real_t mnz = std::min({a.pos.z, b.pos.z, c.pos.z}), mxz = std::max({a.pos.z, b.pos.z, c.pos.z});
        // Outside (sufficient condition => дыр не даст): весь треугольник строго за боксом по оси.
        if (mxx < lo.x || mnx > hi.x) return;
        if (mxy < lo.y || mny > hi.y) return;
        if (mxz < lo.z || mnz > hi.z) return;
        const auto in = [&](const Vertex& v){
            return v.pos.x>=lo.x&&v.pos.x<=hi.x&&v.pos.y>=lo.y&&v.pos.y<=hi.y&&v.pos.z>=lo.z&&v.pos.z<=hi.z;
        };
        crossing = !(in(a) && in(b) && in(c));   // не все внутри => пересекает границу
    }
    const int base = verts.size();
    verts.push_back(a.pos); verts.push_back(b.pos); verts.push_back(c.pos);
    norms.push_back(a.normal); norms.push_back(b.normal); norms.push_back(c.normal);
    uvs.push_back(a.uv); uvs.push_back(b.uv); uvs.push_back(c.uv);
    idx.push_back(base); idx.push_back(base + 1); idx.push_back(base + 2);
    marks.push_back(crossing ? 1 : 0);
}

MeshGenerator::MeshBuilder MeshGenerator::builder(uint64_t shape_id, uint64_t chunk_id) const {
    MeshBuilder b;
    b.filter = false;
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return b;
    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return b;
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return b;
    const AABB g = chunk->get_aabb();
    b.box = AABB(-g.size * 0.5, g.size);
    b.filter = true;
    return b;
}

void MeshGenerator::submit(uint64_t shape_id, uint64_t chunk_id, const MeshBuilder& b) const {
    auto* wm = WorldMesh::get_singleton();
    if (!wm) return;

    Ref<ArrayMesh> mesh; mesh.instantiate();
    if (!b.verts.is_empty()) {
        Array arr; arr.resize(Mesh::ARRAY_MAX);
        arr[Mesh::ARRAY_VERTEX] = b.verts;
        arr[Mesh::ARRAY_NORMAL] = b.norms;
        arr[Mesh::ARRAY_TEX_UV] = b.uvs;
        arr[Mesh::ARRAY_INDEX]  = b.idx;
        mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);
    }

    wm->complete_mesh(shape_id, chunk_id, mesh);
}