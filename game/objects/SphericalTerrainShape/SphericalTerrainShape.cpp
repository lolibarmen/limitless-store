#include "SphericalTerrainShape.hpp"
#include <vector>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
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
    _generator.instantiate();

    _material_generator.instantiate();
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load("res://assets/Chunk/grass.png"));
}

void SphericalTerrainShape::recompute_aabb() {
    _aabb = AABB(_center - Vector3(_radius, _radius, _radius), Vector3(_radius * 2.0f, _radius * 2.0f, _radius * 2.0f));
    _aabb_dirty = false;
}

void SphericalTerrainShape::on_register() {
    // Фигуре нужно собственное октодерево — регистрируем себя в ShapeSpatialManager
    // со своими параметрами: дистанции перехода на более глубокий уровень и размер корня.
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) {
        return;
    }

    const AABB bounds = get_aabb();
    float kRootSize = 2*_radius; // свой размер корневого чанка

    // Дистанции, на которых происходит переход на более глубокий уровень октодерева
    // (по убыванию). Их количество задаёт максимальную глубину детализации.
    std::vector<float> split_distances;
    for (float size = kRootSize; size > 16.0f; size *= 0.5f) {
        split_distances.push_back(size * 4.0f);
    }

    mgr->create_octree(get_id(), bounds.get_center(), kRootSize, split_distances);
}

float SphericalTerrainShape::evaluate_sdf(const Vector3& world_pos) const {
    return world_pos.distance_to(_center) - _radius;
}

void SphericalTerrainShape::on_zone_changed(uint64_t chunk_id) {
    if (WorldMesh* wm = WorldMesh::get_singleton())
        wm->request_render(get_id(), chunk_id, _generator, _material_generator);
}
