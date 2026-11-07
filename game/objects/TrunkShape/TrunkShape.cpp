#include "TrunkShape.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>

using namespace godot;

void TrunkShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_origin", "origin"), &TrunkShape::set_origin);
    ClassDB::bind_method(D_METHOD("get_origin"), &TrunkShape::get_origin);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "origin"), "set_origin", "get_origin");

    ClassDB::bind_method(D_METHOD("set_up", "up"), &TrunkShape::set_up);
    ClassDB::bind_method(D_METHOD("get_up"), &TrunkShape::get_up);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "up"), "set_up", "get_up");

    ClassDB::bind_method(D_METHOD("set_height", "height"), &TrunkShape::set_height);
    ClassDB::bind_method(D_METHOD("get_height"), &TrunkShape::get_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.1,100.0,0.01"), "set_height", "get_height");

    ClassDB::bind_method(D_METHOD("set_base_radius", "radius"), &TrunkShape::set_base_radius);
    ClassDB::bind_method(D_METHOD("get_base_radius"), &TrunkShape::get_base_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_radius", PROPERTY_HINT_RANGE, "0.01,10.0,0.01"), "set_base_radius", "get_base_radius");

    ClassDB::bind_method(D_METHOD("set_top_radius", "radius"), &TrunkShape::set_top_radius);
    ClassDB::bind_method(D_METHOD("get_top_radius"), &TrunkShape::get_top_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "top_radius", PROPERTY_HINT_RANGE, "0.0,10.0,0.01"), "set_top_radius", "get_top_radius");

    ClassDB::bind_method(D_METHOD("set_bend", "bend"), &TrunkShape::set_bend);
    ClassDB::bind_method(D_METHOD("get_bend"), &TrunkShape::get_bend);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "bend"), "set_bend", "get_bend");

    ClassDB::bind_method(D_METHOD("set_radial_segments", "segments"), &TrunkShape::set_radial_segments);
    ClassDB::bind_method(D_METHOD("get_radial_segments"), &TrunkShape::get_radial_segments);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "radial_segments", PROPERTY_HINT_RANGE, "3,32,1"), "set_radial_segments", "get_radial_segments");

    ClassDB::bind_method(D_METHOD("set_height_segments", "segments"), &TrunkShape::set_height_segments);
    ClassDB::bind_method(D_METHOD("get_height_segments"), &TrunkShape::get_height_segments);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "height_segments", PROPERTY_HINT_RANGE, "1,64,1"), "set_height_segments", "get_height_segments");

    ClassDB::bind_method(D_METHOD("set_ring_variation", "variation"), &TrunkShape::set_ring_variation);
    ClassDB::bind_method(D_METHOD("get_ring_variation"), &TrunkShape::get_ring_variation);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ring_variation", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_ring_variation", "get_ring_variation");

    ClassDB::bind_method(D_METHOD("bind_planet", "planet"), &TrunkShape::bind_planet);
    ClassDB::bind_method(D_METHOD("centerline", "t"), &TrunkShape::centerline);
    ClassDB::bind_method(D_METHOD("centerline_tangent", "t"), &TrunkShape::centerline_tangent);
    ClassDB::bind_method(D_METHOD("radius_at", "t"), &TrunkShape::radius_at);
}

TrunkShape::TrunkShape() {
    _mesh_generator.instantiate();
    _material_generator.instantiate();

    Ref<Texture2D> log = ResourceLoader::get_singleton()->load("res://assets/Tree/log.png");
    if (log.is_valid()) {
        _material_generator->set_albedo_texture(log);
    }
    _material_generator->set_texture_scale(2.0f);
    _material_generator->set_roughness(0.9f);
}

void TrunkShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;
    notify_shape_changed();
}

void TrunkShape::set_up(const Vector3& v) {
    const float len = v.length();
    _up = len > 0.0001f ? v / len : Vector3(0.0f, 1.0f, 0.0f);
    notify_shape_changed();
}

Vector3 TrunkShape::centerline(float t) const {
    const float s = CLAMP(t, 0.0f, 1.0f);
    const float smooth = s * s * (3.0f - 2.0f * s);
    return _origin + _up * (_height * s) + _bend * smooth;
}

Vector3 TrunkShape::centerline_tangent(float t) const {
    const float s = CLAMP(t, 0.0f, 1.0f);
    const Vector3 d = _up * _height + _bend * (6.0f * s * (1.0f - s));
    const float len = d.length();
    return len > 0.0001f ? d / len : _up;
}

float TrunkShape::radius_at(float t) const {
    const float s = CLAMP(t, 0.0f, 1.0f);
    return _base_radius + (_top_radius - _base_radius) * s;
}

void TrunkShape::recompute_aabb() {
    const float rmax = Math::max(_base_radius, _top_radius) * (1.0f + _ring_variation) + 0.1f;

    AABB box(centerline(0.0f), Vector3());
    box = box.expand(centerline(1.0f));

    const Vector3 margin(rmax, rmax, rmax);
    _aabb = AABB(box.position - margin, box.size + margin * 2.0f);
    _aabb_dirty = false;
}

void TrunkShape::on_register() {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid()) {
        mgr->bind_spatially(get_id(), _planet->get_id());
    }
}

void TrunkShape::on_zone_changed(uint64_t chunk_id) {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    Chunk* chunk = mgr ? mgr->get_chunk(get_id(), chunk_id) : nullptr;
    if (!chunk) return;
    if (!chunk->get_aabb().has_point(get_aabb().get_center())) return;

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
    }
}
