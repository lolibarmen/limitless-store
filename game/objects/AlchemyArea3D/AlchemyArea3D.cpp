#include "AlchemyArea3D.hpp"
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void AlchemyArea3D::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_commit_alchemy", "self_id", "other_id"), &AlchemyArea3D::_commit_alchemy);
}

AlchemyArea3D::AlchemyArea3D() {
    connect("area_entered", callable_mp(this, &AlchemyArea3D::_on_area_entered));
}

void AlchemyArea3D::_on_area_entered(Area3D* other) {
    if (!other || _shape_id == 0) return;

    AlchemyArea3D* other_area = Object::cast_to<AlchemyArea3D>(other);
    if (!other_area) return;

    const uint64_t other_id = other_area->get_shape_id();
    if (other_id == 0 || other_id == _shape_id) return;

    call_deferred("_commit_alchemy", (int64_t)_shape_id, (int64_t)other_id);
}

void AlchemyArea3D::_commit_alchemy(int64_t self_id, int64_t other_id) {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return;

    const uint64_t self = (uint64_t)self_id;
    Ref<SemanticShape> self_shape = sw->get_shape(self);
    if (self_shape.is_null()) return;

    Ref<SemanticShape> proposed = self_shape->evaluate_alchemy((uint64_t)other_id);
    if (proposed.is_null()) return;
    if (proposed->get_shape_type() == StringName("shape")) return;
    if (proposed.ptr() == self_shape.ptr()) return;
    if (sw->get_shape(self).ptr() != self_shape.ptr()) return;

    sw->propose_shape_change(self, proposed);
}
