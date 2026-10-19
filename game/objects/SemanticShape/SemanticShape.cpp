#include "SemanticShape.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

SemanticShape::SemanticShape() {}

SemanticShape::~SemanticShape() {
}

void SemanticShape::_bind_methods() {
    // Binding'и для Godot, если нужны
}

AABB SemanticShape::get_aabb() {
    if (_aabb_dirty) {
        recompute_aabb();
        _aabb_dirty = false;
    }
    return _aabb;
}

void SemanticShape::notify_shape_changed() {
    _aabb_dirty = true;
}

float SemanticShape::evaluate_sdf(const Vector3& world_pos) const {
    return 1e10f;
}

void SemanticShape::add_owned_shape(uint64_t id) {
    if (!_owned_shape_ids.has(id)) {
        _owned_shape_ids.push_back(id);
    }
}

void SemanticShape::remove_owned_shape(uint64_t id) {
    int idx = _owned_shape_ids.find(id);
    if (idx >= 0) {
        _owned_shape_ids.remove_at(idx);
    }
}

bool SemanticShape::has_owned_shape(uint64_t id) const {
    return _owned_shape_ids.has(id);
}

godot::PackedInt64Array SemanticShape::get_owned_shapes() const {
    return _owned_shape_ids;
}

void SemanticShape::set_owned_shapes(const godot::PackedInt64Array& ids) {
    _owned_shape_ids = ids;
}

void SemanticShape::replace_owned_id(uint64_t old_id, uint64_t new_id) {
    int idx = _owned_shape_ids.find(old_id);
    if (idx >= 0) {
        _owned_shape_ids[idx] = new_id;
    }
}