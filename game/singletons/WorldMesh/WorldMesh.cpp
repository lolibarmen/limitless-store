#include "WorldMesh.hpp"
#include <ChunkOctree/Chunk.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
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

void WorldMesh::request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> generator) {
    if (shape_id == 0 || generator.is_null()) return;

    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;
    
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return; 

    // 1. Очищаем текущий чанк и всех его детей
    _clear_shape_in_subtree(chunk, shape_id);

    // 2. Поднимаемся по родителям и удаляем меш у ПЕРВОГО вхождения
    Chunk* current_parent = chunk->parent;
    while (current_parent) {
        if (current_parent->shape_meshes.count(shape_id) > 0) {
            _remove_mesh_from_chunk(current_parent, shape_id);
            break;
        }
        current_parent = current_parent->parent;
    }

    // 3. Пытаемся сгенерировать меш
    Ref<ArrayMesh> mesh = generator->generate(shape_id, chunk_id);

    if (mesh.is_valid()) {
        MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
        mesh_instance->set_mesh(mesh);
        add_child(mesh_instance);
        mesh_instance->set_global_position(chunk->center);
        
        chunk->shape_meshes[shape_id] = mesh_instance;
    } else {
        MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
        add_child(mesh_instance);
        mesh_instance->set_global_position(chunk->center);

        chunk->shape_meshes[shape_id] = mesh_instance;
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
    if (mesh.is_null()) {
        // print_error("[WorldMesh] ERROR: generator return NULL mesh for chunk_id=", chunk_id);
        return;
    }

    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) {
        return; 
    }

    auto it = chunk->shape_meshes.find(shape_id);
    if (it == chunk->shape_meshes.end()) {
        return;
    }

    MeshInstance3D* mesh_instance = it->second;
    
    mesh_instance->set_mesh(mesh);
}