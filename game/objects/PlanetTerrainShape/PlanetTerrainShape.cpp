#include "PlanetTerrainShape.hpp"
#include <PlanetShape/PlanetShape.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>

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

float PlanetTerrainShape::evaluate_sdf(const Vector3& world_pos) const {
    // Единственный источник SDF — PlanetShape: базовое значение берётся оттуда.
    // Фигура «видна» только внутри своей ячейки Вороного (своего материала),
    // вне её — INFINITY, чтобы не конкурировать с другими материалами.
    const float base_sdf = _planet.is_valid() ? _planet->evaluate_sdf(world_pos) : INFINITY;

    if (_shared_voronoi.is_null()) {
        return base_sdf;
    }

    const Vector3 d = world_pos - _center;
    // Вороной тоже берётся по направлению (нормализованный вектор), а не по полной точке:
    // иначе границы материалов «плывут» вместе с удалением от центра.
    const Vector3 dir = d.length() > 0.0f ? d.normalized() : Vector3(0.0f, 1.0f, 0.0f);
    const float cell_id = _shared_voronoi->get_noise_3dv(dir);

    if (cell_id == static_cast<float>(_this_material_id)) {
        return base_sdf;
    } else {
        return INFINITY;
    }
}

void PlanetTerrainShape::on_zone_changed(uint64_t chunk_id) {
    if (WorldMesh* wm = WorldMesh::get_singleton())
        wm->request_render(get_id(), chunk_id, _generator, _material_generator);
}
