#pragma once
#include "Chunk.hpp"
#include <godot_cpp/variant/vector3i.hpp>
#include <unordered_map>
#include <memory>
#include <functional>
#include <Utils/SpatialHash.hpp>

namespace godot {

class ChunkOctree : public Object {
    GDCLASS(ChunkOctree, Object)

private:
    std::unordered_map<Vector3i, std::unique_ptr<Chunk>, Vector3iHash> roots;
    std::unordered_map<Vector3i, Chunk*, Vector3iHash> leaf_chunks;

    // Внутренний помощник для рекурсивной очистки (используется при удалении корня)
    void destroy_subtree(Chunk* node);

protected:
    static void _bind_methods() {};

public:
    ChunkOctree() = default;
    ~ChunkOctree() { clear(); }

    static ChunkOctree* get_singleton();

    // Управление корнями
    void add_root(const Vector3i& key, const Vector3& center, float size);
    void remove_root(const Vector3i& key);
    
    bool has_root(const Vector3i& key) const { return roots.count(key) > 0; }
    void for_each_root(std::function<void(Chunk*)> func) {
        for (auto& [key, ptr] : roots) {
            if (ptr) func(ptr.get());
        }
    }

    // Операции над структурой дерева
    // Разделяет узел на 8 детей. Узел должен быть листом.
    void split_node(Chunk* node);
    
    // Удаляет всех детей узла, превращая его обратно в лист.
    void collapse_node(Chunk* node);

    // Доступ к текущим активным листьям (для рендеринга или уведомлений)
    const std::unordered_map<Vector3i, Chunk*, Vector3iHash>& get_leaf_chunks() const {
        return leaf_chunks;
    }

    // Полная очистка дерева
    void clear();
};

} // namespace godot