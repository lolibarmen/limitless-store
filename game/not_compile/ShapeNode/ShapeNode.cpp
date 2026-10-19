#include "ShapeNode.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ShapeNode::ShapeNode(uint64_t key, uint8_t d, const AABB& aabb)
    : key(key), depth(d), local_aabb(aabb), geometry_dirty(true), 
      mesh_instance(nullptr), mesh_requested(false) {}

void ShapeNode::request_mesh(WorldMesh* wm, Ref<MaterialGenerator> material_gen) {
    if (!wm || mesh_requested) return;
    
    // Создаём MeshInstance3D через WorldMesh
    mesh_instance = wm->create_instance();
    if (!mesh_instance) {
        ERR_PRINT("ShapeNode::request_mesh: Failed to create MeshInstance3D");
        return;
    }
    
    pending_material = material_gen;
    mesh_requested = true;
}

void ShapeNode::release_mesh(WorldMesh* wm) {
    if (!mesh_instance || !wm) return;
    
    wm->destroy_instance(mesh_instance);
    mesh_instance = nullptr;
    pending_material = Ref<MaterialGenerator>();
    mesh_requested = false;
}

void ShapeNode::apply_mesh(Ref<ArrayMesh> mesh) {
    if (!mesh_instance || mesh.is_null()) return;
    
    mesh_instance->set_mesh(mesh);
    geometry_dirty = false;
}

void ShapeNode::apply_material(Ref<Material> material) {
    if (!mesh_instance || material.is_null()) return;
    
    if (mesh_instance->get_mesh().is_valid() && mesh_instance->get_mesh()->get_surface_count() > 0) {
        mesh_instance->set_surface_override_material(0, material);
    }
    
    pending_material = Ref<MaterialGenerator>();
}