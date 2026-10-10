#include "WorldMesh.hpp"
#include <ChunkOctree/Chunk.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/engine.hpp>
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
    chunk->remove_shape_mesh(shape_id);
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
    if (shape_id == 0 || mesh_generator.is_null() || material_generator.is_null()) return;

    auto* octree = ChunkOctree::get_singleton();
    
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return;

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
    
    Ref<ArrayMesh> mesh = mesh_generator->generate(shape_id, chunk_id);

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);
    // mesh_instance->set_global_position(chunk->center);
    
    if (mesh.is_valid() && mesh->get_surface_count() > 0) {
        mesh_instance->set_mesh(mesh);
        
        Ref<Material> material = material_generator->generate(shape_id, chunk_id);;
        if (material.is_valid()) {
            mesh_instance->set_surface_override_material(0, material);
        }
        
        chunk->shape_meshes[shape_id] = mesh_instance;
    } else {
        chunk->shape_meshes[shape_id] = mesh_instance;
        chunk->shape_material_generators[shape_id] = material_generator;
    }    
}

void WorldMesh::cancel_render(uint64_t shape_id, uint64_t chunk_id) {
    if (shape_id == 0) return;

    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    // Пытаемся найти чанк. 
    Chunk* chunk = octree->find(chunk_id);
    if (chunk) {
        // Если чанк жив, удаляем меш из него (и вызываем queue_free)
        chunk->remove_shape_mesh(shape_id);
    }
}

void WorldMesh::complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh) {
    if (mesh.is_null()) return;

    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return; 

    auto it = chunk->shape_meshes.find(shape_id);
    if (it == chunk->shape_meshes.end()) {
        return; // Чанк удален или свернут, результат неактуален
    }

    MeshInstance3D* mesh_instance = it->second;
    
    mesh_instance->set_mesh(mesh);

    if (mesh->get_surface_count() > 0) {
        
        // 3. Берем генератор материала, который фигура выбрала для этого LOD
        auto mat_it = chunk->shape_material_generators.find(shape_id);
        if (mat_it != chunk->shape_material_generators.end() && mat_it->second.is_valid()) {
            
            // 4. Генерируем материал (для StandardMaterial3D это мгновенно)
            Ref<Material> material = mat_it->second->generate(shape_id, chunk_id);
            
            if (material.is_valid()) {
                mesh_instance->set_surface_override_material(0, material);
            }
        }
    }
}