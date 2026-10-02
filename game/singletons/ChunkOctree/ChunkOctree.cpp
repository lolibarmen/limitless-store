#include "ChunkOctree.hpp"
#include "Chunk.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot {

ChunkOctree* ChunkOctree::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("ChunkOctree");
    return Object::cast_to<ChunkOctree>(obj);
}

void ChunkOctree::destroy_subtree(Chunk* node) {
    if (!node) return;

    // Удаляем из карт ДО физического уничтожения дочерних элементов
    id_to_chunk.erase(node->id);
    if (node->is_leaf()) {
        leaf_chunks.erase(node);
    }
    
    node->clear_debug_mesh();

    if (!node->is_leaf()) {
        for (auto& child_ptr : node->children) {
            if (child_ptr) {
                destroy_subtree(child_ptr.get());
                child_ptr.reset(); // Уникальный указатель автоматически удалит объект
            }
        }
    }
}

uint64_t ChunkOctree::add_root(const Vector3& center, float size) {
    uint64_t new_id = _next_id++;
    // Используем new, так как в заголовке roots хранит сырые указатели (Chunk*)
    Chunk* root = new Chunk(new_id, center, size, 0, nullptr);
    
    roots[new_id] = root;
    leaf_chunks.insert(root);
    id_to_chunk[new_id] = root;
    
    return new_id;
}

void ChunkOctree::remove_root(uint64_t id) {
    auto it = roots.find(id);
    if (it != roots.end()) {
        Chunk* root = it->second;
        destroy_subtree(root);
        roots.erase(it);
        delete root; // Освобождаем память корневого элемента
    }
}

void ChunkOctree::for_each_root(std::function<void(Chunk*)> func) {
    for (auto& pair : roots) {
        if (pair.second) {
            func(pair.second);
        }
    }
}

void ChunkOctree::split(Chunk* node) {
    if (!node || !node->is_leaf()) return;

    leaf_chunks.erase(node);

    float h = node->size / 2.0f;
    float q = h / 2.0f;
    int i = 0;
    
    // Иерархическая генерация ID для детей
    for (int x : {-1, 1}) {
        for (int y : {-1, 1}) {
            for (int z : {-1, 1}) {
                uint64_t child_id = (node->id << 3) | i;
                
                node->children[i] = std::make_unique<Chunk>(
                    child_id, 
                    node->center + Vector3(x, y, z) * q, 
                    h, 
                    node->depth + 1, 
                    node
                );
                
                Chunk* new_child = node->children[i].get();
                leaf_chunks.insert(new_child);
                id_to_chunk[child_id] = new_child;
                i++;
            }
        }
    }
}

void ChunkOctree::collapse(Chunk* node) {
    if (!node || node->is_leaf()) return;
    
    for (auto& child_ptr : node->children) {
        if (child_ptr) {
            destroy_subtree(child_ptr.get());
            child_ptr.reset();
        }
    }
    
    leaf_chunks.insert(node);
}

void ChunkOctree::for_each_leaf(std::function<void(Chunk*)> func) {
    for (Chunk* leaf : leaf_chunks) {
        if (leaf) {
            func(leaf);
        }
    }
}

Chunk* ChunkOctree::find(uint64_t id) const {
    auto it = id_to_chunk.find(id);
    if (it != id_to_chunk.end()) {
        return it->second;
    }
    return nullptr;
}

void ChunkOctree::clear() {
    for (auto& pair : roots) {
        destroy_subtree(pair.second);
        delete pair.second;
    }
    roots.clear();
    leaf_chunks.clear();
    id_to_chunk.clear();
    _next_id = 1;
}

} // namespace godot