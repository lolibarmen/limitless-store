#include "WorldMesh.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/shape3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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

void WorldMesh::_remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id) {
    if (!chunk) return;
    chunk->remove_shape(shape_id);
}

void WorldMesh::_clear_shape_in_subtree(Chunk* node, uint64_t shape_id) {
    if (!node) return;
    
    _remove_mesh_from_chunk(node, shape_id);

    for (auto& child_ptr : node->children) {
        if (child_ptr) {
            _clear_shape_in_subtree(child_ptr.get(), shape_id);
        }
    }
}

void WorldMesh::request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> mesh_generator, Ref<MaterialGenerator> material_generator) {
    if (shape_id == 0 || mesh_generator.is_null() || material_generator.is_null()) {
        UtilityFunctions::printerr("WorldMesh::request_render: Неверные входные данные.");
        return;
    }

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) { UtilityFunctions::printerr("WorldMesh::request_render: ShapeSpatialManager не найден."); return; }
    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) { UtilityFunctions::printerr("WorldMesh::request_render: Октодерево для shape_id ", shape_id, " не найдено! Фигура не зарегистрирована в ShapeSpatialManager (см. on_register/create_octree)."); return; }
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) { UtilityFunctions::printerr("WorldMesh::request_render: Чанк ", chunk_id, " не найден в октодереве фигуры ", shape_id, "."); return; }

    _clear_shape_in_subtree(chunk, shape_id);
    Chunk* current_parent = chunk->parent;
    while (current_parent) {
        if (current_parent->shape_meshes.count(shape_id) > 0) {
            _remove_mesh_from_chunk(current_parent, shape_id);
            current_parent->shape_material_generators.erase(shape_id);
            break;
        }
        current_parent = current_parent->parent;
    }

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);
    mesh_instance->set_global_position(chunk->center);
    chunk->shape_meshes[shape_id] = mesh_instance;

    StaticBody3D* collision_body = memnew(StaticBody3D);
    add_child(collision_body);
    collision_body->set_global_position(chunk->center);

    CollisionShape3D* collision_shape = memnew(CollisionShape3D);
    collision_body->add_child(collision_shape);

    Chunk::ShapeCollision collision;
    collision.body = collision_body;
    collision.shape_node = collision_shape;
    chunk->shape_collisions[shape_id] = collision;

    chunk->shape_material_generators[shape_id] = material_generator;
    
    mesh_generator->generate(shape_id, chunk_id);
}

void WorldMesh::cancel_render(uint64_t shape_id, uint64_t chunk_id) {
    if (shape_id == 0) return;
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (octree) {
        Chunk* chunk = octree->find(chunk_id);
        if (chunk) chunk->remove_shape(shape_id);
    }
}

void WorldMesh::complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh, Ref<ConcavePolygonShape3D> collision_shape) {
    if (mesh.is_null()) return;
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return; 

    auto it = chunk->shape_meshes.find(shape_id);
    if (it == chunk->shape_meshes.end()) return;

    MeshInstance3D* mesh_instance = it->second;
    mesh_instance->set_mesh(mesh);

    auto collision_it = chunk->shape_collisions.find(shape_id);
    if (collision_it != chunk->shape_collisions.end() && collision_it->second.shape_node) {
        if (mesh->get_surface_count() > 0) {
            Ref<ConcavePolygonShape3D> shape = collision_shape;
            if (shape.is_null()) {
                shape = mesh->create_trimesh_shape();
            }
            collision_it->second.shape_node->set_shape(shape);
        } else {
            collision_it->second.shape_node->set_shape(Ref<Shape3D>());
        }
    }

    if (mesh->get_surface_count() > 0) {
        auto mat_it = chunk->shape_material_generators.find(shape_id);
        if (mat_it != chunk->shape_material_generators.end() && mat_it->second.is_valid()) {
            Ref<Material> material = mat_it->second->generate(shape_id, chunk_id);
            if (material.is_valid()) {
                mesh_instance->set_surface_override_material(0, material);
            }
        }
    }
}