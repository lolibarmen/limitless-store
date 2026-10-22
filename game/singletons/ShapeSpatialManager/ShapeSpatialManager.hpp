#pragma once

#include "ChunkOctree.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <atomic>
#include <cstdint>

namespace godot {

/// @brief Глобальный менеджер пространственного разбиения.
/// Управляет созданием ChunkOctree для фигур и обеспечивает наследование пространственной сетки.
class ShapeSpatialManager : public Object {
    GDCLASS(ShapeSpatialManager, Object)

private:
    /// Деревья, которыми этот менеджер владеет (созданы для is_relative == true или корневых фигур)
    std::unordered_map<uint64_t, std::unique_ptr<ChunkOctree>> _owned_octrees;
    
    /// Пространственная привязка: для каждой фигуры указывает, чье дерево она использует.
    /// Может указывать на собственное дерево или на дерево родителя.
    std::unordered_map<uint64_t, ChunkOctree*> _shape_to_octree;
    
    std::atomic<uint64_t> _next_chunk_id{1};

protected:
    static void _bind_methods() {}

public:
    static ShapeSpatialManager* get_singleton();

    /// @brief Регистрирует фигуру и автоматически решает, нужно ли ей собственное октодерево.
    void register_shape(uint64_t shape_id, Ref<SemanticShape> shape);

    /// @brief Связывает дочернюю фигуру с пространственной сеткой родительской.
    void bind_spatially(uint64_t child_shape_id, uint64_t parent_shape_id);

    /// @brief Удаляет фигуру и корректно очищает ссылки на её дерево у дочерних элементов.
    void unregister_shape(uint64_t shape_id);

    /// @brief Возвращает октодерево, которое должна использовать данная фигура.
    ChunkOctree* get_octree(uint64_t shape_id) const;

    /// @brief Итерация только по тем фигурам, которые владеют собственным октодеревом.
    void for_each_owned_octree(std::function<void(uint64_t shape_id, ChunkOctree* octree)> func);

    /// @brief Глобальный провайдер уникальных ID для всех чанков в сцене.
    uint64_t get_next_chunk_id() { return _next_chunk_id++; }
};

} // namespace godot