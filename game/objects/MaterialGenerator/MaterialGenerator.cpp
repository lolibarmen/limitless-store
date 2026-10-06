#include "MaterialGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MaterialGenerator::_bind_methods() {}

Ref<Material> MaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    return Ref<Material>();
}