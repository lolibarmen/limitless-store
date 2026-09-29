#include "WorldMesh.hpp"
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

using namespace godot;

void WorldMesh::_bind_methods() {}

WorldMesh* WorldMesh::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("WorldMesh");
    return Object::cast_to<WorldMesh>(obj);
}

void WorldMesh::_ready() {}
void WorldMesh::_process(double delta) {}

void WorldMesh::_cleanup_shape_meshes(uint64_t shape_id) {
    auto it = _shape_meshes.find(shape_id);
    if (it != _shape_meshes.end()) {
        for (auto* mesh_inst : it->second) {
            if (mesh_inst) {
                mesh_inst->queue_free();
            }
        }
        _shape_meshes.erase(it);
    }
}

void WorldMesh::request_render(uint64_t shape_id, const AABB& bounds) {
    Ref<SemanticShape> shape = SemanticWorld::get_singleton()->get_shape(shape_id);
    if (shape.is_null()) return;

    Ref<MeshGenerator> generator = shape->get_mesh_generator();
    if (generator.is_null()) return;

    generator->generate(shape_id, bounds); 
}

void WorldMesh::request_render(uint64_t shape_id, const AABB& bounds, Ref<MeshGenerator> generator) {
    if (generator.is_null()) return;
    
    Ref<SemanticShape> shape = SemanticWorld::get_singleton()->get_shape(shape_id);
    if (shape.is_null()) return;

    generator->generate(shape_id, bounds); 
}

void WorldMesh::cancel_render(uint64_t shape_id, const AABB& bounds) {
    auto it = _shape_meshes.find(shape_id);
    if (it == _shape_meshes.end()) return;

    Vector3 target_center = bounds.get_center();
    float threshold = bounds.size.length() * 0.1f; // Допуск

    for (auto mesh_it = it->second.begin(); mesh_it != it->second.end(); ) {
        MeshInstance3D* inst = *mesh_it;

        if (inst && inst->is_inside_tree() && inst->get_global_position().distance_to(target_center) < threshold) {
            inst->queue_free();
            mesh_it = it->second.erase(mesh_it);
        } else {
            ++mesh_it;
        }
    }
}

void WorldMesh::complete_mesh(uint64_t shape_id, const AABB& bounds, Ref<ArrayMesh> mesh) {
    if (mesh.is_null()) {
        print_line(vformat("[WorldMesh] ERROR: generator return NULL mesh for zone_center=%s", bounds.get_center()));
        return;
    }

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    mesh_instance->set_mesh(mesh);

    add_child(mesh_instance);

    mesh_instance->set_global_position(bounds.get_center());
    
    _shape_meshes[shape_id].push_back(mesh_instance);
}