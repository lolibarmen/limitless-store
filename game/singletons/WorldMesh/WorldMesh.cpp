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
    
    // КЛЮЧЕВОЙ МОМЕНТ: find вернет nullptr, если чанк был уничтожен Octree.
    // Это наша главная защита от висячих указателей.
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return; 

    // 1. LOD ВВЕРХ (Укрупнение): удаляем меши из подчанков
    _clear_shape_in_subtree(chunk, shape_id);

    // 2. LOD ВНИЗ (Уточнение): удаляем меш из родителя, если он есть
    Chunk* current_parent = chunk->parent;
    while (current_parent) {
        if (current_parent->shape_meshes.count(shape_id) > 0) {
            _remove_mesh_from_chunk(current_parent, shape_id);
            break;
        }
        current_parent = current_parent->parent;
    }

    // 3. Запуск генерации
    generator->generate(shape_id, chunk_id); 
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
    // Если chunk == nullptr, значит ChunkOctree уже уничтожил этот чанк.
    // В этом случае ничего делать не нужно: деструктор ~Chunk() уже вызвал 
    // queue_free() для всех мешей в shape_meshes. Утечки не будет.
}

void WorldMesh::complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh) {
    if (mesh.is_null()) {
        print_line(vformat("[WorldMesh] ERROR: generator return NULL mesh for chunk_id=%llu", chunk_id));
        return;
    }

    auto* octree = ChunkOctree::get_singleton();
    if (!octree) return;

    // Снова проверяем, жив ли чанк. За время работы генератора его могли схлопнуть.
    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) {
        // Чанк уничтожен. Просто игнорируем результат генерации. 
        // Mesh будет автоматически удален сборщиком мусора Godot, так как мы его никуда не добавили.
        return; 
    }

    MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
    mesh_instance->set_mesh(mesh);
    add_child(mesh_instance);
    mesh_instance->set_global_position(chunk->center);

    // Chunk становится единственным владельцем и источником истины об этом меше
    chunk->shape_meshes[shape_id] = mesh_instance;
}