#include "SphericalTerrainShape.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SphericalTerrainShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_center", "center"), &SphericalTerrainShape::set_center);
    ClassDB::bind_method(D_METHOD("get_center"), &SphericalTerrainShape::get_center);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");

    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphericalTerrainShape::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &SphericalTerrainShape::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
}

SphericalTerrainShape::SphericalTerrainShape() {
    _is_relative = true;
    _generator.instantiate();

    _material_generator.instantiate();
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load("res://assets/Chunk/grass.png"));
}

void SphericalTerrainShape::recompute_aabb() {
    _aabb = AABB(_center - Vector3(_radius, _radius, _radius), Vector3(_radius * 2.0f, _radius * 2.0f, _radius * 2.0f));
    _aabb_dirty = false;
}

float SphericalTerrainShape::evaluate_sdf(const Vector3& world_pos) const {
    return world_pos.distance_to(_center) - _radius;
}

void SphericalTerrainShape::on_zone_changed(uint64_t chunk_id) {
    if (WorldMesh* wm = WorldMesh::get_singleton())
        wm->request_render(get_id(), chunk_id, _generator, _material_generator);
}
