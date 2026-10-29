#pragma once

#include "Chunk.hpp"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace godot {

class ChunkOctree {
    
private:
    std::unordered_map<uint64_t, Chunk*> roots;
    std::unordered_set<Chunk*> leaf_chunks;
    std::unordered_map<uint64_t, Chunk*> id_to_chunk;

    /// @brief Фигуры, подписанные на события активации чанков этого дерева.
    std::unordered_set<uint64_t> _users;

    /// @brief Активные leaf-чанки дерева в предыдущем кадре обновления LOD.
    std::unordered_set<uint64_t> _prev_active_chunks;

    /// @brief Пользователи, подключённые после прошлого обновления; им отправляются все текущие чанки.
    std::unordered_set<uint64_t> _pending_users;

    void destroy_subtree(Chunk* node);

    /// @brief Рекурсивно оценивает и обновляет LOD для конкретного узла и его потомков.
    void evaluate_node(Chunk* node, const Vector3& viewer_pos);
    
public:
    /// @brief Уникальный ID дерева, выдаваемый ShapeSpatialManager.
    uint64_t id = 0;

    ~ChunkOctree() { clear(); }

    /// @brief Добавляет фигуру в пользователи дерева и помечает её как нуждающуюся в текущих чанках.
    void add_user(uint64_t shape_id);

    /// @brief Удаляет фигуру из пользователей дерева.
    void remove_user(uint64_t shape_id);

    /// @brief Возвращает количество пользователей дерева.
    size_t user_count() const;

    /// @brief Выполняет func для каждого пользователя дерева.
    void for_each_user(std::function<void(uint64_t shape_id)> func) const;

    /// @brief Константы октодерева (в одном месте):
    /// - CHUNK_BASE_SIZE — базовый (минимальный) размер чанка, не зависит от AABB фигуры;
    /// - MAX_DEPTH — максимальная глубина разбиения.
    /// Размер корня всегда равен CHUNK_BASE_SIZE, глубина разбиения ограничена MAX_DEPTH.
    static constexpr int CHUNK_BASE_SIZE = 16;
    static constexpr int MAX_DEPTH = 8;

    uint64_t add_root(const Vector3& center);
    void remove_root(uint64_t id);
    
    void for_each_root(std::function<void(Chunk*)> func);

    void split(Chunk* node);
    void collapse(Chunk* node);
    
    /// @brief Автоматически оценивает и обновляет LOD дерева на основе позиции наблюдателя.
    void evaluate_and_update_lod(const Vector3& viewer_pos);

    /// @brief Обновляет LOD дерева и уведомляет пользователей о активированных для них чанках.
    void update_lod_and_notify(
        const Vector3& viewer_pos,
        const std::function<void(uint64_t shape_id, const std::vector<uint64_t>& chunk_ids)>& notify
    );

    void for_each_leaf(std::function<void(Chunk*)> func);

    Chunk* find(uint64_t id) const;

    void clear();
};

} // namespace godot