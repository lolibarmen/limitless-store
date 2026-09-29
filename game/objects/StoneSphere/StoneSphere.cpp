#include "StoneSphere.hpp"
#include <WorldMesh/WorldMesh.hpp>

using namespace godot;

void StoneSphere::_bind_methods() {

}

StoneSphere::StoneSphere() {
    _generator.instantiate();
}

Ref<MeshGenerator> StoneSphere::get_mesh_generator() const {
    return _generator;
}

void StoneSphere::on_zone_changed(const AABB& zone, int lod_level) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) return;

    // Если LOD валиден (>= 0), запрашиваем отрисовку в этой зоне
    if (lod_level >= 0) {
        wm->request_render(get_id(), zone);
    } 
    // Если зона выгружена (LOD_UNLOADED), отменяем отрисовку, чтобы очистить память
    else {
        wm->cancel_render(get_id(), zone);
    }
}