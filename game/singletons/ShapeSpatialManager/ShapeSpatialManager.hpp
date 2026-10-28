#pragma once

#include "ChunkOctree.hpp"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/engine.hpp>

#include <SemanticShape/SemanticShape.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

namespace godot {

/// @brief Глобальный менеджер пространственного разбиения.
/// Управляет созданием ChunkOctree для фигур и обеспечивает наследование пространственной сетки.
class ShapeSpatialManager : public Object {
    GDCLASS(ShapeSpatialManager, Object)

private:
    /// Все октодеревья, которыми владеет менеджер. Ключ — уникальный ID дерева.
    std::unordered_map<uint64_t, std::unique_ptr<ChunkOctree>> _octrees;

    /// Пространственная привязка фигур. Ключ — ID фигуры, значение — ID используемого дерева.
    /// Отсутствие записи означает, что у фигуры нет октодерева.
    std::unordered_map<uint64_t, uint64_t> _shape_octree;

    /// Счётчик уникальных ID октодеревьев.
    std::atomic<uint64_t> _next_octree_id{1};

    /// Глобальный счётчик уникальных ID чанков.
    std::atomic<uint64_t> _next_chunk_id{1};

    /// @brief Создаёт октодерево и делает фигуру его первым пользователем.
    uint64_t create_octree(uint64_t shape_id, const AABB& bounds);

    /// @brief Привязывает фигуру к существующему дереву.
    void attach_shape(uint64_t shape_id, uint64_t octree_id);

    /// @brief Отвязывает фигуру от дерева; уничтожает дерево, если пользователей не осталось.
    void detach_shape(uint64_t shape_id);

    /// @brief Уничтожает дерево и очищает привязки всех его пользователей.
    void destroy_octree(uint64_t octree_id);

protected:
    static void _bind_methods() {}

public:
    static ShapeSpatialManager* get_singleton();

    /// @brief Регистрирует фигуру и, если is_relative() == true, создаёт ей собственное октодерево.
    /// @details Иначе фигура не получает дерево до bind_spatially().
    void register_shape(uint64_t shape_id, Ref<SemanticShape> shape);

    /// @brief Связывает фигуру с пространственной сеткой другой фигуры.
    /// @details Родительская фигура должна быть зарегистрирована раньше дочерней.
    void bind_spatially(uint64_t child_shape_id, uint64_t parent_shape_id);

    /// @brief Удаляет фигуру из пространственной привязки и, при необходимости, уничтожает её дерево.
    void unregister_shape(uint64_t shape_id);

    /// @brief Возвращает октодерево, которое должна использовать данная фигура.
    ChunkOctree* get_octree(uint64_t shape_id) const;

    /// @brief Возвращает чанк дерева фигуры по его ID.
    /// @details Нужен маленьким фигурам для проверки центра AABB внутри чанка.
    Chunk* get_chunk(uint64_t shape_id, uint64_t chunk_id) const;

    /// @brief Итерация по всем живым октодеревьям.
    /// @details Callback не должен уничтожать текущее дерево.
    void for_each_octree(std::function<void(uint64_t octree_id, ChunkOctree* octree)> func);

    /// @brief Глобальный провайдер уникальных ID для всех чанков в сцене.
    uint64_t get_next_chunk_id() { return _next_chunk_id++; }
};

} // namespace godot