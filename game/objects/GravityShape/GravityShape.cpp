#include "GravityShape.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GravityShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("bind_planet", "planet"), &GravityShape::bind_planet);
    ClassDB::bind_method(D_METHOD("get_planet"), &GravityShape::get_planet);
    ClassDB::bind_method(D_METHOD("get_up_at", "local_pos"), &GravityShape::get_up_at);
    ClassDB::bind_method(D_METHOD("get_area_radius"), &GravityShape::get_area_radius);
}

GravityShape::GravityShape() {
    _area_generator.instantiate();
}

void GravityShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;
    notify_shape_changed();
}

Vector3 GravityShape::get_up_at(const Vector3& local_pos) const {
    const Vector3 d = local_pos - _position;
    const float len = d.length();
    if (len > 0.0001f) {
        return d / len;
    }
    return Vector3(0.0f, 1.0f, 0.0f);
}

float GravityShape::get_area_radius() const {
    if (_planet.is_null()) {
        return 1.0f;
    }
    return _planet->get_radius() + _planet->get_amplitude();
}

void GravityShape::recompute_aabb() {
    if (_planet.is_valid()) {
        _aabb = _planet->get_aabb();
    } else {
        _aabb = AABB();
    }
    _aabb_dirty = false;
}

void GravityShape::on_register() {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid() && _planet->get_id() != 0) {
        mgr->bind_spatially(get_id(), _planet->get_id());
    }
}

void GravityShape::on_zone_changed(uint64_t chunk_id) {
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    Chunk* chunk = mgr ? mgr->get_chunk(get_id(), chunk_id) : nullptr;
    if (!chunk) return;
    if (!chunk->get_aabb().has_point(get_aabb().get_center())) return;

    WorldMesh* wm = WorldMesh::get_singleton();
    if (wm) {
        wm->request_render(get_id(), chunk_id, Ref<MeshGenerator>(), Ref<MeshGenerator>(),
                           _area_generator, Ref<MaterialGenerator>());
    }
}
