#include "TestSphere.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>

using namespace godot;

void TestSphere::_bind_methods() {

}

TestSphere::TestSphere() {
    _mesh_generator_lod0.instantiate();
    _mesh_generator_lod0->set_lod_level(0);
    _mesh_generator_lod1.instantiate();
    _mesh_generator_lod1->set_lod_level(1);
    _mesh_generator_lod2.instantiate();
    _mesh_generator_lod2->set_lod_level(2);

    _material_generator.instantiate();
}

void TestSphere::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) {
        return;
    }

    Chunk* chunk = ChunkOctree::get_singleton()->find(chunk_id);
    if (!chunk || !chunk->is_leaf()) {
        return;
    }

    Ref<MeshGenerator> active_generator;
    switch (chunk->depth) {
        case 2:
            active_generator = _mesh_generator_lod0;
            break;
        case 1:
            active_generator = _mesh_generator_lod1;
            break;
        case 0:
            active_generator = _mesh_generator_lod2;
            break;
        default:
            return;
    }

    if (active_generator.is_null() || _material_generator.is_null()) {
        return;
    }
    
    wm->request_render(get_id(), chunk_id, active_generator, _material_generator);
}