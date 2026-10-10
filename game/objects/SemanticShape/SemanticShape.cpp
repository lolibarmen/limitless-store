#include "SemanticShape.hpp"
#include <MeshGenerator/MeshGenerator.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SemanticShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_id"), &SemanticShape::get_id);
    ClassDB::bind_method(D_METHOD("get_shape_type"), &SemanticShape::get_shape_type);
    ClassDB::bind_method(D_METHOD("get_aabb"), &SemanticShape::get_aabb);
}

SemanticShape::SemanticShape() = default;
SemanticShape::~SemanticShape() = default;

AABB SemanticShape::get_aabb() {
    if (_aabb_dirty) {
        recompute_aabb();
        _aabb_dirty = false;
    }
    return _aabb;
}

void SemanticShape::notify_shape_changed() {
    recompute_aabb();
}

float SemanticShape::evaluate_sdf(const Vector3& world_pos) const {
    return 1e10f;
}

void SemanticShape::add_owned_shape(uint64_t id) {
    if (!_owned_shape_ids.has(id)) {
        _owned_shape_ids.push_back(id);
        notify_shape_changed();
    }
}

void SemanticShape::remove_owned_shape(uint64_t id) {
    int64_t idx = _owned_shape_ids.find(id);
    if (idx != -1) {
        _owned_shape_ids.remove_at(idx);
        notify_shape_changed();
    }
}

bool SemanticShape::has_owned_shape(uint64_t id) const {
    return _owned_shape_ids.has(id);
}

PackedInt64Array SemanticShape::get_owned_shapes() const {
    return _owned_shape_ids;
}

void SemanticShape::set_owned_shapes(const PackedInt64Array& ids) {
    if (_owned_shape_ids != ids) {
        _owned_shape_ids = ids;
        notify_shape_changed();
    }
}

void SemanticShape::replace_owned_id(uint64_t old_id, uint64_t new_id) {
    int64_t idx = _owned_shape_ids.find(old_id);
    if (idx != -1) {
        _owned_shape_ids[idx] = new_id;
        notify_shape_changed();
    }
}