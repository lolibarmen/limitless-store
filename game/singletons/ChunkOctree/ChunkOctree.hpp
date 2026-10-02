#pragma once
#include "Chunk.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <cstdint>

namespace godot {

class ChunkOctree : public Object {
    GDCLASS(ChunkOctree, Object)

private:
    uint64_t _next_id = 1;

    std::unordered_map<uint64_t, Chunk*> roots;
    std::unordered_set<Chunk*> leaf_chunks;
    std::unordered_map<uint64_t, Chunk*> id_to_chunk;

    void destroy_subtree(Chunk* node);

protected:
    static void _bind_methods() {};

public:
    ChunkOctree() = default;
    ~ChunkOctree() { clear(); }

    static ChunkOctree* get_singleton();

    uint64_t add_root(const Vector3& center, float size);
    void remove_root(uint64_t id);
    
    void for_each_root(std::function<void(Chunk*)> func);

    void split(Chunk* node);
    void collapse(Chunk* node);

    void for_each_leaf(std::function<void(Chunk*)> func);

    Chunk* find(uint64_t id) const;

    void clear();
};

} // namespace godot