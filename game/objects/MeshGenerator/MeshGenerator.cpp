#include "MeshGenerator.hpp"
#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MeshGenerator::_bind_methods() {}

Ref<ArrayMesh> MeshGenerator::generate(Ref<SemanticShape> shape) const {
    return Ref<ArrayMesh>();
}