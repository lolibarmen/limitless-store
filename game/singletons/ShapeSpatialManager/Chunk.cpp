#include "Chunk.hpp"
#include <WorldMesh/RenderRequest.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

Chunk::Chunk(uint64_t id, Vector3 c, float s, int d, Chunk* p)
    : id(id), center(c), size(s), depth(d), parent(p) {}

Chunk::~Chunk() {
    clear_all_shape_meshes();
}

AABB Chunk::get_aabb() const {
    float safe_size = Math::abs(size);
    float half = safe_size / 2.0f;
    return AABB(center - Vector3(half, half, half), Vector3(safe_size, safe_size, safe_size));
}

void Chunk::remove_shape(uint64_t shape_id) {
    if (WorldMesh* wm = WorldMesh::get_singleton()) {
        wm->cancel_request(shape_id, id);
    }

    auto it = shape_requests.find(shape_id);
    if (it != shape_requests.end()) {
        it->second->drop();
        shape_requests.erase(it);
    }
}

void Chunk::clear_all_shape_meshes() {
    WorldMesh* wm = WorldMesh::get_singleton();

    for (auto& pair : shape_requests) {
        if (wm) {
            wm->cancel_request(pair.first, id);
        }
        pair.second->drop();
    }
    shape_requests.clear();
}
