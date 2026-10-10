#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <cstdint>

namespace godot {

/// @brief Глобальный менеджер семантических фигур.
/// Управляет их жизненным циклом, графом зависимостей и транзакционными изменениями состояния.
class SemanticWorld : public Object {
    GDCLASS(SemanticWorld, Object)

private:
    std::unordered_map<uint64_t, Ref<SemanticShape>> _shapes;
    Ref<Mutex> _shapes_mutex;
    std::atomic<uint64_t> _next_id{1};

protected:
    static void _bind_methods();

public:
    SemanticWorld();
    ~SemanticWorld() override = default;

    /// @brief Возвращает глобальный экземпляр SemanticWorld, зарегистрированный в Godot Engine.
    static SemanticWorld* get_singleton();

    /// @brief Регистрирует новую фигуру в мире, присваивает ей уникальный ID и возвращает его.
    /// @param shape Ссылка на фигуру (должна быть валидной).
    /// @return Уникальный ID зарегистрированной фигуры. Поточно-безопасен.
    uint64_t register_shape(Ref<SemanticShape> shape);

    /// @brief Удаляет фигуру из мира по её ID. Поточно-безопасен.
    /// @param id Уникальный ID удаляемой фигуры.
    void unregister_shape(uint64_t id);

    /// @brief Возвращает ссылку на фигуру по её ID.
    /// @param id Уникальный ID фигуры.
    /// @return Ref на фигуру. Возвращает невалидный Ref, если фигура не найдена. Поточно-безопасен.
    Ref<SemanticShape> get_shape(uint64_t id) const;

    /// @brief Уведомляет фигуры об изменении состояния чанков (зон).
    /// Используется системой LOD для обновления состояния фигур при загрузке/выгрузке областей.
    /// @param chunk_ids Вектор ID чанков, состояние которых изменилось.
    void on_zone_changed(const std::vector<uint64_t>& chunk_ids);

    /// @brief Инициирует транзакционное изменение состояния фигуры.
    /// Запускает каскадную оценку реакций (evaluate_reaction) у зависимых объектов и атомарно
    /// применяет все одобренные изменения, обновляя граф зависимостей.
    /// @param old_id ID фигуры, которая должна быть заменена.
    /// @param proposed_shape Предложение (Ref) новой, еще не зарегистрированной фигуры.
    void propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape);
};

} // namespace godot