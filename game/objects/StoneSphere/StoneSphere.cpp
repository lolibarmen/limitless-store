#include "StoneSphere.hpp"
#include <WorldMesh/WorldMesh.hpp>

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

void StoneSphere::on_zone_changed(const AABB& zone, int lod_level) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) return;

    if (lod_level < 0) {
        wm->cancel_render(get_id(), zone);
        return;
    }

    wm->cancel_render(get_id(), zone);

    switch (lod_level)
    {
    case 0:
        wm->request_render(get_id(), zone, _generator_lod0);
        break;
    case 1:
        wm->request_render(get_id(), zone, _generator_lod1);
        break;
    case 2:
        wm->request_render(get_id(), zone, _generator_lod2);
        break;
    default:
        break;
    }
}