#include "MeshGenerator.hpp"
#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MeshGenerator::_bind_methods() {}

Ref<ArrayMesh> MeshGenerator::generate(uint64_t shape_id, const AABB& bounds) const {
    return Ref<ArrayMesh>();
}