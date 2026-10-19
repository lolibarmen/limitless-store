#include "StarSystem.hpp"
#include <WorldMesh/WorldMesh.hpp>

using namespace godot;

void StarSystem::_bind_methods() {}

StarSystem::StarSystem() {
    _mesh_generator.instantiate();
    _material_generator.instantiate();
    _mesh_generator->set_pos(_pos);
    _mesh_generator->set_radius(_radius);
}

void StarSystem::recompute_aabb() {
    _aabb = AABB(_pos - Vector3(_radius, _radius, _radius), Vector3(_radius * 2.0f, _radius * 2.0f, _radius * 2.0f));
}

void StarSystem::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm || _mesh_generator.is_null() || _material_generator.is_null()) return;
    
    wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
}