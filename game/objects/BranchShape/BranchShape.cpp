#include "BranchShape.hpp"
#include "MaterialGenerator/MaterialGenerator.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void BranchShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_position", "position"), &BranchShape::set_position);
    ClassDB::bind_method(D_METHOD("get_position"), &BranchShape::get_position);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

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
    ClassDB::bind_method(D_METHOD("lean"), &BranchShape::lean);
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

void BranchShape::set_direction(const Vector3& v) {
    const float len = v.length();
    _direction = len > 0.0001f ? v / len : Vector3(0.0f, 1.0f, 0.0f);
    notify_shape_changed();
}

Vector3 BranchShape::up() const {
    if (_planet.is_valid()) {
        const Vector3 nose = _position - _planet->get_center();
        const float len = nose.length();
        if (len > 0.0001f) {
            return nose / len;
        }
    }
    return Vector3(0.0f, 1.0f, 0.0f);
}

Vector3 BranchShape::side() const {
    const Vector3 u = up();
    const Vector3 w = _direction - u * _direction.dot(u);
    const float len = w.length();
    if (len > 0.0001f) {
        return w / len;
    }
    const Vector3 helper = Math::abs(u.y) < 0.99f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    return u.cross(helper).normalized();
}

float BranchShape::card_height() const {
    return Math::max(0.0001f, _length * _direction.dot(up()));
}

float BranchShape::lean() const {
    return _length * _direction.dot(side());
}

float BranchShape::card_width() const {
    return _width + Math::abs(lean());
}

void BranchShape::recompute_aabb() {
    const Vector3 s = side() * (card_width() * 0.5f);
    const Vector3 tip = _position + up() * card_height();

    AABB box(_position - s, Vector3());
    box = box.expand(_position + s);
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
        wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
    }
}
