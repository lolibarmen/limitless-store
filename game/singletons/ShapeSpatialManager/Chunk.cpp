#include "Chunk.hpp"
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

void Chunk::remove_shape_mesh(uint64_t shape_id) {
    auto it = shape_meshes.find(shape_id);
    if (it != shape_meshes.end()) {
        if (it->second) {
            it->second->queue_free();
        }
        shape_meshes.erase(it);
    }
}

void Chunk::remove_shape_collision(uint64_t shape_id) {
    auto it = shape_collisions.find(shape_id);
    if (it != shape_collisions.end()) {
        if (it->second.body) {
            it->second.body->queue_free();
        } else if (it->second.shape_node) {
            it->second.shape_node->queue_free();
        }
        shape_collisions.erase(it);
    }
}

void Chunk::remove_shape(uint64_t shape_id) {
    remove_shape_mesh(shape_id);
    remove_shape_collision(shape_id);
}

void Chunk::clear_all_shape_meshes() {
    for (auto& pair : shape_meshes) {
        if (pair.second) {
            pair.second->queue_free();
        }
    }
    shape_meshes.clear();

    for (auto& pair : shape_collisions) {
        if (pair.second.body) {
            pair.second.body->queue_free();
        } else if (pair.second.shape_node) {
            pair.second.shape_node->queue_free();
        }
    }
    shape_collisions.clear();
}