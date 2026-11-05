#include "PlanetShape.hpp"
#include "godot_cpp/core/math.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void PlanetShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_center", "center"), &PlanetShape::set_center);
    ClassDB::bind_method(D_METHOD("get_center"), &PlanetShape::get_center);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");

    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &PlanetShape::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &PlanetShape::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

    ClassDB::bind_method(D_METHOD("set_amplitude", "amplitude"), &PlanetShape::set_amplitude);
    ClassDB::bind_method(D_METHOD("get_amplitude"), &PlanetShape::get_amplitude);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");
}

PlanetShape::PlanetShape() {
    _noise.instantiate();
    _noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX);
    _noise->set_fractal_octaves(2);

    _shared_voronoi.instantiate();
    _shared_voronoi->set_noise_type(FastNoiseLite::TYPE_CELLULAR);
    _shared_voronoi->set_cellular_return_type(FastNoiseLite::RETURN_CELL_VALUE);
    
    _mesh_generator.instantiate();
    _material_generator.instantiate();
}

int PlanetShape::get_material_id(const Vector3& world_pos) const {
    // Используем направление от центра, как и в evaluate_sdf, 
    // чтобы биомы не "плыли" по вертикали вместе с рельефом.
    const Vector3 d = world_pos - _center;
    const float len = d.length();
    const Vector3 dir = len > 0.0f ? d / len : Vector3(0.0f, 1.0f, 0.0f);
    
    float val = _shared_voronoi->get_noise_3dv(dir);
    // Нормализуем значение Cellular Noise в диапазон [0, 1]
    float normalized = (val + 1.0f) * 0.5f;
    normalized = CLAMP(normalized, 0.0f, 1.0f);
    
    const int kMat = 2;
    return 5;
    return (int)(normalized * kMat);
}

float PlanetShape::evaluate_sdf(const Vector3& world_pos) const {
    const Vector3 d = world_pos - _center;
    // Шум берётся по направлению (нормализованный вектор), а не по полной точке:
    // иначе высота зависит от расстояния до центра и радиус «плывёт» вместе с удалением.
    const float len = d.length();
    const Vector3 dir = len > 0.0f ? d / len : Vector3(0.0f, 1.0f, 0.0f);
    const float height = _radius + _noise->get_noise_3dv(dir) * _amplitude;
    return len - height;
}

void PlanetShape::paint_vertices(const Vector3* positions, const Vector3* normals, Color* out_colors, int count, const Vector3& posoff) const {
    for (int i = 0; i < count; ++i) {
        Vector3 world_pos = positions[i] + posoff;
        
        int biome_id = get_material_id(world_pos);
        out_colors[i] = Color((float)biome_id / 7.0f, 0.0f, 0.0f, 1.0f);
    }
}

void PlanetShape::recompute_aabb() {
    const float r = _radius + _amplitude;
    _aabb = AABB(_center - Vector3(r, r, r), Vector3(r * 2.0f, r * 2.0f, r * 2.0f));
    _aabb_dirty = false;
}

void PlanetShape::on_register() {
    _noise->set_seed(get_id());
    _shared_voronoi->set_seed(get_id());
    _noise->set_frequency(_radius / 100.0f);
    _shared_voronoi->set_frequency(_radius / 100.0f);

    SemanticWorld* sw = SemanticWorld::get_singleton();
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!sw || !mgr) {
        return;
    }

    // Собственное октодерево планеты.
    const float r = _radius + _amplitude;
    constexpr int kMaxDepth = 6;
    mgr->create_octree(get_id(), _center, r * 2.0f, kMaxDepth);
}

void PlanetShape::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
}