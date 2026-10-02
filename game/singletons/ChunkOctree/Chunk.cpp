#include "Chunk.hpp"
#include <godot_cpp/core/math.hpp>

using namespace godot;

Chunk::Chunk(uint64_t id, Vector3 c, float s, int d, Chunk* p)
    : id(id), center(c), size(s), depth(d), parent(p) {}

Chunk::~Chunk() {
    clear_debug_mesh();
    // Очищаем все меши, принадлежащие этому чанку, при его уничтожении
    for (auto& pair : shape_meshes) {
        if (pair.second) {
            pair.second->queue_free();
        }
    }
    shape_meshes.clear();
}

AABB Chunk::get_aabb() const {
    float safe_size = Math::abs(size);
    float half = safe_size / 2.0f;
    return AABB(center - Vector3(half, half, half), Vector3(safe_size, safe_size, safe_size));
}

void Chunk::clear_debug_mesh() {
    if (debug_mesh) {
        debug_mesh->queue_free();
        debug_mesh = nullptr;
    }
}

void Chunk::remove_shape_mesh(uint64_t shape_id) {
    auto it = shape_meshes.find(shape_id);
    if (it != shape_meshes.end()) {
        if (it->second) {
            it->second->queue_free();
        }
        shape_meshes.erase(it);
    }
}