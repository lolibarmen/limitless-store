#include "TestCube.hpp"
#include <vector>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void TestCube::_bind_methods() {
    // Binding'и для Godot, если нужны
}

TestCube::TestCube() {
    _pos = Vector3(-10.0, -10.0, -10.0);
    _size = Vector3(20.0, 20.0, 20.0);

    _mesh_generator.instantiate();

    _material_generator.instantiate();
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load("res://assets/test_texture.jpg"));
}

void TestCube::recompute_aabb() {
    _aabb = AABB(_pos, _size);
}

void TestCube::on_register() {
    // Фигуре нужно собственное октодерево — регистрируем себя в ShapeSpatialManager
    // со своими параметрами: дистанции перехода на более глубокий уровень и размер корня.
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) {
        return;
    }

    const AABB bounds = get_aabb();
    constexpr float kRootSize = 64.0f; // свой размер корневого чанка

    // Дистанции, на которых происходит переход на более глубокий уровень октодерева
    // (по убыванию). Их количество задаёт максимальную глубину детализации.
    std::vector<float> split_distances;
    for (float size = kRootSize; size > 16.0f; size *= 0.5f) {
        split_distances.push_back(size * 4.0f);
    }

    mgr->create_octree(get_id(), bounds.get_center(), kRootSize, split_distances);
}

void TestCube::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) {
        return;
    }

    if (_mesh_generator.is_null() || _material_generator.is_null()) {
        return;
    }

    wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
}