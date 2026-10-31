#include "PlanetTerrainShape.hpp"
#include <PlanetShape/PlanetShape.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <cmath>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void PlanetTerrainShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_center", "center"), &PlanetTerrainShape::set_center);
    ClassDB::bind_method(D_METHOD("get_center"), &PlanetTerrainShape::get_center);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");

    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &PlanetTerrainShape::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &PlanetTerrainShape::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

    ClassDB::bind_method(D_METHOD("set_amplitude", "amplitude"), &PlanetTerrainShape::set_amplitude);
    ClassDB::bind_method(D_METHOD("get_amplitude"), &PlanetTerrainShape::get_amplitude);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");
}

PlanetTerrainShape::PlanetTerrainShape() {
    _noise.instantiate();
    _noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX);
    _noise->set_frequency(1.0f);

    _generator.instantiate();

    _material_generator.instantiate();
}

void PlanetTerrainShape::recompute_aabb() {
    const float r = _radius + _amplitude;
    _aabb = AABB(_center - Vector3(r, r, r), Vector3(r * 2.0f, r * 2.0f, r * 2.0f));
    _aabb_dirty = false;
}

void PlanetTerrainShape::on_register() {
    _noise->set_seed(_id);
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load(_albedo_texture));
}

int PlanetTerrainShape::material_at(const Vector3& dir) const {
    if (_shared_voronoi.is_null()) {
        return 0;
    }

    const float n = _shared_voronoi->get_noise_3dv(dir);
    float t = (n + 1.0f) * 0.5f;
    t = Math::clamp(t, 0.0f, 1.0f);
    
    int mat_count = 3;
    int idx = int(Math::floor(t * mat_count));
    idx = Math::clamp(idx, 0, mat_count - 1);
    
    return idx;
}

float PlanetTerrainShape::evaluate_sdf(const Vector3& world_pos) const {
    // Единственный источник SDF — PlanetShape: базовое значение берётся оттуда.
    // Фигура «видна» только внутри своей ячейки Вороного (своего материала),
    // вне её — INFINITY, чтобы не конкурировать с другими материалами.
    const float base_sdf = _planet.is_valid() ? _planet->evaluate_sdf(world_pos) : INFINITY;

    if (_shared_voronoi.is_null()) {
        return base_sdf;
    }
    
    const Vector3 d = world_pos - _center;
    const Vector3 dir = d.length() > 0.0f ? d.normalized() : Vector3(0.0f, 1.0f, 0.0f);

    if (material_at(dir) == _this_material_id) {
        return base_sdf;
    } else {        
        return 1;
    }
}

void PlanetTerrainShape::on_zone_changed(uint64_t chunk_id) {
    if (WorldMesh* wm = WorldMesh::get_singleton())
        wm->request_render(get_id(), chunk_id, _generator, _material_generator);
}