#include "BranchShape.hpp"
#include "MaterialGenerator/MaterialGenerator.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void BranchShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_up", "up"), &BranchShape::set_up);
    ClassDB::bind_method(D_METHOD("get_up"), &BranchShape::get_up);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "up"), "set_up", "get_up");

    ClassDB::bind_method(D_METHOD("set_direction", "direction"), &BranchShape::set_direction);
    ClassDB::bind_method(D_METHOD("get_direction"), &BranchShape::get_direction);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction"), "set_direction", "get_direction");

    ClassDB::bind_method(D_METHOD("set_length", "length"), &BranchShape::set_length);
    ClassDB::bind_method(D_METHOD("get_length"), &BranchShape::get_length);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_RANGE, "0.01,50.0,0.01"), "set_length", "get_length");

    ClassDB::bind_method(D_METHOD("set_width", "width"), &BranchShape::set_width);
    ClassDB::bind_method(D_METHOD("get_width"), &BranchShape::get_width);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width", PROPERTY_HINT_RANGE, "0.01,10.0,0.01"), "set_width", "get_width");

    ClassDB::bind_method(D_METHOD("bind_planet", "planet"), &BranchShape::bind_planet);
    ClassDB::bind_method(D_METHOD("up"), &BranchShape::up);
    ClassDB::bind_method(D_METHOD("side"), &BranchShape::side);
    ClassDB::bind_method(D_METHOD("card_height"), &BranchShape::card_height);
    ClassDB::bind_method(D_METHOD("card_width"), &BranchShape::card_width);
}

BranchShape::BranchShape() {
    _mesh_generator.instantiate();
    _material_generator.instantiate();
}

void BranchShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;
    notify_shape_changed();
}

void BranchShape::set_up(const Vector3& v) {
    const float len = v.length();
    const Vector3 normalized = len > 0.0001f ? v / len : Vector3(0.0f, 1.0f, 0.0f);
    if (_up == normalized) {
        return;
    }
    _up = normalized;
    notify_shape_changed();
}

void BranchShape::set_direction(const Vector3& v) {
    const float len = v.length();
    _direction = len > 0.0001f ? v / len : Vector3(0.0f, 1.0f, 0.0f);
    notify_shape_changed();
}

void BranchShape::set_fork_count(int v) {
    const int clamped = CLAMP(v, 0, 6);
    if (_material_generator->get_fork_count() == clamped) {
        return;
    }
    _material_generator->set_fork_count(clamped);
    notify_shape_changed();
}

int BranchShape::get_fork_count() const {
    return _material_generator->get_fork_count();
}

Vector3 BranchShape::up() const {
    return _up;
}

Vector3 BranchShape::side() const {
    const Vector3 u = up();
    const Vector3 w = _direction - u * _direction.dot(u);
    const float len = w.length();
    if (len > 0.0001f) {
        return w / len;
    }
    const Vector3 a = u.abs();
    const Vector3 helper = a.x <= a.y && a.x <= a.z ? Vector3(1.0f, 0.0f, 0.0f)
                         : a.y <= a.z                ? Vector3(0.0f, 1.0f, 0.0f)
                                                     : Vector3(0.0f, 0.0f, 1.0f);
    return u.cross(helper).normalized();
}

float BranchShape::card_height() const {
    return Math::max(0.0001f, _length * _direction.dot(up()));
}

float BranchShape::card_width() const {
    const float lean = _length * _direction.dot(side());
    return Math::max(0.0001f, _width + Math::abs(lean));
}

void BranchShape::recompute_aabb() {
    const Vector3 s = side() * (card_width() * 0.5f);
    const Vector3 tip = get_position() + up() * card_height();

    AABB box(get_position() - s, Vector3());
    box = box.expand(get_position() + s);
    box = box.expand(tip - s);
    box = box.expand(tip + s);

    const Vector3 margin(0.01f, 0.01f, 0.01f);
    _aabb = AABB(box.position - margin, box.size + margin * 2.0f);
    _aabb_dirty = false;
}

void BranchShape::on_register() {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid()) {
        mgr->bind_spatially(get_id(), _planet->get_id());
    }
}

void BranchShape::on_zone_changed(uint64_t chunk_id) {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    Chunk* chunk = mgr ? mgr->get_chunk(get_id(), chunk_id) : nullptr;
    if (!chunk) return;
    if (!chunk->get_aabb().has_point(get_aabb().get_center())) return;

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->request_render(get_id(), chunk_id, _mesh_generator,
                           Ref<MeshGenerator>(), Ref<MeshGenerator>(), _material_generator);
    }
}
