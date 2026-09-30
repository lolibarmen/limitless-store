#include "ChunkOctree.hpp"
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

ChunkOctree* ChunkOctree::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("ChunkOctree");
    return Object::cast_to<ChunkOctree>(obj);
}

void ChunkOctree::destroy_subtree(Chunk* node) {
    if (!node || node->is_leaf()) return;
    
    for (auto& child_ptr : node->children) {
        if (child_ptr) {
            Chunk* child = child_ptr.get();
            if (child->is_leaf()) {
                Vector3i child_key((int)child->center.x, (int)child->center.y, (int)child->center.z);
                leaf_chunks.erase(child_key);
            }
            child->clear_debug_mesh();
            destroy_subtree(child);
            child_ptr.reset();
        }
    }
}

void ChunkOctree::add_root(const Vector3i& key, const Vector3& center, float size) {
    auto root = std::make_unique<Chunk>(center, size, 0, nullptr);
    Chunk* raw = root.get();
    roots[key] = std::move(root);
    leaf_chunks[key] = raw;
}

void ChunkOctree::remove_root(const Vector3i& key) {
    auto it = roots.find(key);
    if (it != roots.end()) {
        destroy_subtree(it->second.get());
        it->second->clear_debug_mesh();
        leaf_chunks.erase(key);
        roots.erase(it);
    }
}

void ChunkOctree::split_node(Chunk* node) {
    if (!node || !node->is_leaf()) return;

    Vector3i parent_key((int)node->center.x, (int)node->center.y, (int)node->center.z);
    leaf_chunks.erase(parent_key);
    node->clear_debug_mesh();

    float h = node->size / 2.0f;
    float q = h / 2.0f;
    int i = 0;
    for (int x : {-1, 1}) {
        for (int y : {-1, 1}) {
            for (int z : {-1, 1}) {
                auto child = std::make_unique<Chunk>(node->center + Vector3(x, y, z) * q, h, node->depth + 1, node);
                Chunk* new_child = child.get();
                
                node->children[i] = std::move(child);
                
                Vector3i child_key((int)new_child->center.x, (int)new_child->center.y, (int)new_child->center.z);
                leaf_chunks[child_key] = new_child;
                i++;
            }
        }
    }
}

void ChunkOctree::collapse_node(Chunk* node) {
    if (!node || node->is_leaf()) return;
    
    destroy_subtree(node);
    // После удаления детей узел снова становится листом
    Vector3i key((int)node->center.x, (int)node->center.y, (int)node->center.z);
    leaf_chunks[key] = node;
}

void ChunkOctree::clear() {
    for (auto& [key, root_ptr] : roots) {
        destroy_subtree(root_ptr.get());
        root_ptr->clear_debug_mesh();
    }
    roots.clear();
    leaf_chunks.clear();
}