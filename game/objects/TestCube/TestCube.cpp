#include "TestCube.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

using namespace godot;

void TestCube::_bind_methods() {

}

TestCube::TestCube() {
    _pos = Vector3(-10.0, -10.0, -10.0);
    _size = Vector3( 20.0,  20.0,  20.0);

    _mesh_generator.instantiate();
    _mesh_generator->set_pos(_pos);
    _mesh_generator->set_size(_size);

    _material_generator.instantiate();
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load("res://assets/test_texture.png"));
}

void TestCube::recompute_aabb() {
    _aabb = AABB(_pos, _size);
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