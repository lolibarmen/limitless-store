#pragma once
#include "Chunk.hpp"
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <cstdint>

namespace godot {

class ChunkOctree {
    
private:
    std::unordered_map<uint64_t, Chunk*> roots;
    std::unordered_set<Chunk*> leaf_chunks;
    std::unordered_map<uint64_t, Chunk*> id_to_chunk;

    void destroy_subtree(Chunk* node);

    /// @brief Рекурсивно оценивает и обновляет LOD для конкретного узла и его потомков.
    void evaluate_node(Chunk* node, const Vector3& viewer_pos, int max_depth);
    
public:
    ~ChunkOctree() { clear(); }

    uint64_t add_root(const Vector3& center, float size);
    void remove_root(uint64_t id);
    
    void for_each_root(std::function<void(Chunk*)> func);

    void split(Chunk* node);
    void collapse(Chunk* node);
    
    /// @brief Автоматически оценивает и обновляет LOD дерева на основе позиции наблюдателя.
    void evaluate_and_update_lod(const Vector3& viewer_pos, int max_depth);

    void for_each_leaf(std::function<void(Chunk*)> func);

    Chunk* find(uint64_t id) const;

    void clear();
};

} // namespace godot