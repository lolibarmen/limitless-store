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

    /// @brief Каскад оценки реакций при фундаментальной замене фигуры.
    /// @details Обход идёт строго по owned-спискам: у заменяемой фигуры опрашиваются
    /// только фигуры, которых она явно владеет. Родитель не вызывается, если он не указан в owned.
    void _run_change_cascade(uint64_t old_id, Ref<SemanticShape> proposed_shape);

    /// @brief Атомарно применяет одобренные замены фигур и обновляет граф зависимостей.
    void _commit_changes(const std::unordered_map<uint64_t, Ref<SemanticShape>>& accepted_changes);

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

    /// @brief Рекурсивно удаляет фигуру и все фигуры, которыми она владеет.
    /// @details Сначала удаляются дети, затем сама фигура. Повторные ID игнорируются.
    void unregister_shape_with_owned(uint64_t id);

    /// @brief Возвращает ссылку на фигуру по её ID.
    /// @param id Уникальный ID фигуры.
    /// @return Ref на фигуру. Возвращает невалидный Ref, если фигура не найдена. Поточно-безопасен.
    Ref<SemanticShape> get_shape(uint64_t id) const;

    /// @brief Уведомляет конкретную фигуру об изменении состояния её чанков (зон).
    /// @param shape_id ID фигуры, чьи чанки изменились.
    /// @param chunk_ids Вектор ID добавленных чанков.
    void on_zone_changed(uint64_t shape_id, const std::vector<uint64_t>& chunk_ids);

    /// @brief Единственная точка входа для фундаментальной замены фигуры.
    /// Запускает каскадную оценку реакций (evaluate_reaction) по owned-спискам и атомарно
    /// применяет все одобренные замены, обновляя граф зависимостей.
    /// @param old_id ID фигуры, которая должна быть заменена.
    /// @param proposed_shape Предложение (Ref) новой, еще не зарегистрированной фигуры.
    /// @details Простые изменения внутренних параметров сюда не передаются: фигура сама
    /// уведомляет свои owned-фигуры через notify_shape_changed().
    void propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape);
};

} // namespace godot