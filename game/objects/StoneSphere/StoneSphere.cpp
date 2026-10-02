#include "StoneSphere.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <ChunkOctree/ChunkOctree.hpp>

using namespace godot;

void StoneSphere::_bind_methods() {

}

StoneSphere::StoneSphere() {
    _generator_lod0.instantiate();
    _generator_lod0->set_lod_level(0);
    _generator_lod1.instantiate();
    _generator_lod1->set_lod_level(1);
    _generator_lod2.instantiate();
    _generator_lod2->set_lod_level(2);
}

Ref<MeshGenerator> StoneSphere::get_mesh_generator() const {
    return _generator_lod0;
}

void StoneSphere::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) return;

    Chunk* chunk = ChunkOctree::get_singleton()->find(chunk_id);
    if(!chunk) return;

    Ref<MeshGenerator> active_generator;
    switch (chunk->depth) {
        case 2:
            active_generator = _generator_lod0;
            break;
        case 1:
            active_generator = _generator_lod1;
            break;
        case 0:
            active_generator = _generator_lod2;
            break;
        default:
            return;
    }

    if (active_generator.is_null()) {
        return;
    }

    wm->request_render(get_id(), chunk_id, active_generator);
}