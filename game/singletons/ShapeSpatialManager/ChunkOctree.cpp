#include "ChunkOctree.hpp"
#include "Chunk.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp> // Для глобального счетчика ID
#include <godot_cpp/core/math.hpp>

namespace godot {

static float cube_distance(const Vector3& a, const Vector3& b) {
    return Math::max(Math::max(Math::abs(a.x - b.x), Math::abs(a.y - b.y)), Math::abs(a.z - b.z));
}

void ChunkOctree::destroy_subtree(Chunk* node) {
    if (!node) return;

    id_to_chunk.erase(node->id);
    leaf_chunks.erase(node);

    for (auto& child_ptr : node->children) {
        if (child_ptr) {
            destroy_subtree(child_ptr.get());
            child_ptr.reset();
        }
    }
}

uint64_t ChunkOctree::add_root(const Vector3& center, float size) {
    uint64_t new_id = ShapeSpatialManager::get_singleton()->get_next_chunk_id();

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
        delete root;
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
    id_to_chunk.erase(node->id);
    node->clear_all_shape_meshes(); 

    float h = node->size / 2.0f;
    float q = h / 2.0f;
    int i = 0;
    
    for (int x : {-1, 1}) {
        for (int y : {-1, 1}) {
            for (int z : {-1, 1}) {
                // ИСПОЛЬЗУЕМ ГЛОБАЛЬНЫЙ СЧЕТЧИК ДЛЯ УНИКАЛЬНОСТИ ID ВО ВСЕЙ СЦЕНЕ
                uint64_t child_id = ShapeSpatialManager::get_singleton()->get_next_chunk_id();
                
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
    id_to_chunk[node->id] = node;
}

void ChunkOctree::evaluate_node(Chunk* node, const Vector3& viewer_pos, int max_depth) {
    if (!node) return;

    float dist = cube_distance(node->center, viewer_pos);
    bool should_split = dist < node->size * 4.0f;
    bool should_collapse = dist > node->size * 4.1f;

    if (node->is_leaf() && should_split && node->depth < max_depth) {
        split(node);
        // Рекурсивно оцениваем только что созданных детей
        for (auto& child_ptr : node->children) {
            if (child_ptr) {
                evaluate_node(child_ptr.get(), viewer_pos, max_depth);
            }
        }
    }
    else if (!node->is_leaf() && should_collapse) {
        collapse(node);
        // После коллапса узел стал листом, рекурсия остановится сама
    }
    else if (!node->is_leaf()) {
        // Если узел уже разделен, рекурсивно оцениваем его детей
        for (auto& child_ptr : node->children) {
            if (child_ptr) {
                evaluate_node(child_ptr.get(), viewer_pos, max_depth);
            }
        }
    }
}

void ChunkOctree::evaluate_and_update_lod(const Vector3& viewer_pos, int max_depth) {
    // Проходим только по корням, а рекурсию делает evaluate_node
    for_each_root([this, &viewer_pos, max_depth](Chunk* root) {
        evaluate_node(root, viewer_pos, max_depth);
    });
}

void ChunkOctree::for_each_leaf(std::function<void(Chunk*)> func) {
    for (Chunk* leaf : leaf_chunks) {
        if (leaf) func(leaf);
    }
}

Chunk* ChunkOctree::find(uint64_t id) const {
    auto it = id_to_chunk.find(id);
    return (it != id_to_chunk.end()) ? it->second : nullptr;
}

void ChunkOctree::clear() {
    for (auto& pair : roots) {
        destroy_subtree(pair.second);
        delete pair.second;
    }
    roots.clear();
    leaf_chunks.clear();
    id_to_chunk.clear();
}

} // namespace godot