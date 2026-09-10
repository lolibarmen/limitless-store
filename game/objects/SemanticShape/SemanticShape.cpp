#include "SemanticShape.hpp"
#include <MeshGenerator/MeshGenerator.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SemanticShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_id"), &SemanticShape::get_id);
    ClassDB::bind_method(D_METHOD("get_shape_type"), &SemanticShape::get_shape_type);
    ClassDB::bind_method(D_METHOD("get_aabb"), &SemanticShape::get_aabb);
}

SemanticShape::SemanticShape() = default;
SemanticShape::~SemanticShape() = default;

AABB SemanticShape::get_aabb() const {
    if (_aabb_dirty) {
        const_cast<SemanticShape*>(this)->recompute_aabb();
        const_cast<SemanticShape*>(this)->_aabb_dirty = false;
    }
    return _aabb;
}

void SemanticShape::notify_shape_changed() {
    recompute_aabb();
    
    if (_generator.is_valid()) {
        _generator->on_shape_changed(this);
    }
    
    if (_world != nullptr) {
        _world->mark_shape_dirty(_id);
    }
}

float SemanticShape::evaluate_sdf(const Vector3& world_pos) const {
    return 1e10f; 
}

void SemanticShape::set_generator(Ref<MeshGenerator> generator) {
    _generator = generator;
}

Ref<MeshGenerator> SemanticShape::get_generator() const {
    return _generator;
}