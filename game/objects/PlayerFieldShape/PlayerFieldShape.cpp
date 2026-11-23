#include "PlayerFieldShape.hpp"
#include <GravityShape/GravityShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void PlayerFieldShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_area_radius", "radius"), &PlayerFieldShape::set_area_radius);
    ClassDB::bind_method(D_METHOD("get_area_radius"), &PlayerFieldShape::get_area_radius);
    ClassDB::bind_method(D_METHOD("get_bound_gravity_id"), &PlayerFieldShape::get_bound_gravity_id);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "area_radius"), "set_area_radius", "get_area_radius");
}

PlayerFieldShape::PlayerFieldShape() {
    _area_generator.instantiate();
}

void PlayerFieldShape::set_area_radius(float r) {
    _area_radius = Math::max(r, 0.001f);
    notify_shape_changed();
}

void PlayerFieldShape::recompute_aabb() {
    const float r = _area_radius;
    _aabb = AABB(Vector3(-r, -r, -r), Vector3(r * 2.0f, r * 2.0f, r * 2.0f));
    _aabb_dirty = false;
}

void PlayerFieldShape::on_register() {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    const float size = Math::max(_area_radius * 4.0f, 16.0f);
    mgr->create_octree(get_id(), get_position(), size, 0);
}

void PlayerFieldShape::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) return;

    wm->request_render(get_id(), chunk_id, Ref<MeshGenerator>(), Ref<MeshGenerator>(),
                       _area_generator, Ref<MaterialGenerator>());
}

Ref<SemanticShape> PlayerFieldShape::evaluate_alchemy(uint64_t other_id) const {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return nullptr;

    Ref<SemanticShape> other = sw->get_shape(other_id);
    if (Object::cast_to<GravityShape>(other.ptr())) {
        _pending_gravity_id = other_id;
    }
    return nullptr;
}

uint64_t PlayerFieldShape::consume_pending_gravity_id() {
    const uint64_t pending = _pending_gravity_id;
    _pending_gravity_id = 0;
    if (pending == 0) return 0;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return 0;

    Ref<SemanticShape> shape = sw->get_shape(pending);
    if (!Object::cast_to<GravityShape>(shape.ptr())) return 0;

    _bound_gravity_id = pending;
    return _bound_gravity_id;
}

void PlayerFieldShape::on_alchemy_separation(uint64_t other_id) {
    if (_pending_gravity_id == other_id) {
        _pending_gravity_id = 0;
    }
    if (_bound_gravity_id == other_id) {
        _bound_gravity_id = 0;
    }
}
