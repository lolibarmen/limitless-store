#include "Chunk.hpp"
#include <godot_cpp/variant/aabb.hpp>

using namespace godot;

Chunk::Chunk(Vector3 c, float s, int d, Chunk* p)
    : center(c), size(s), depth(d), parent(p) {}

Chunk::~Chunk() {
    clear_debug_mesh();
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