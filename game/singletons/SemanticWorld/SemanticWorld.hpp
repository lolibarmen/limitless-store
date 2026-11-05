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

    bool _parameter_change_in_progress = false;

    /// @brief Единый каскад оценки реакций для замены фигуры или изменения её параметров.
    /// @details В режиме замены обходит владельцев изменяемой фигуры, как раньше.
    /// В режиме изменения параметров обходит граф владения в обе стороны, уведомляет
    /// связанные фигуры и не заменяет исходную фигуру.
    void _run_change_cascade(uint64_t old_id, Ref<SemanticShape> proposed_shape, bool parameter_change);

    /// @brief Возвращает фигуры, связанные с shape_id через граф владения.
    /// @param two_way Если true, учитываются и владельцы shape_id, и фигуры, которыми она владеет.
    std::vector<uint64_t> _find_related_shapes(uint64_t shape_id, bool two_way) const;

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

    /// @brief Возвращает ссылку на фигуру по её ID.
    /// @param id Уникальный ID фигуры.
    /// @return Ref на фигуру. Возвращает невалидный Ref, если фигура не найдена. Поточно-безопасен.
    Ref<SemanticShape> get_shape(uint64_t id) const;

    /// @brief Уведомляет конкретную фигуру об изменении состояния её чанков (зон).
    /// @param shape_id ID фигуры, чьи чанки изменились.
    /// @param chunk_ids Вектор ID добавленных чанков.
    void on_zone_changed(uint64_t shape_id, const std::vector<uint64_t>& chunk_ids);

    /// @brief Инициирует транзакционное изменение состояния фигуры.
    /// Запускает каскадную оценку реакций (evaluate_reaction) у зависимых объектов и атомарно
    /// применяет все одобренные изменения, обновляя граф зависимостей.
    /// @param old_id ID фигуры, которая должна быть заменена.
    /// @param proposed_shape Предложение (Ref) новой, еще не зарегистрированной фигуры.
    /// @details Если proposed_shape — это та же фигура, что зарегистрирована под old_id,
    /// изменение трактуется как изменение параметров существующей фигуры: ID сохраняется,
    /// а связанные фигуры получают уведомление и могут вернуть собственные реакции.
    void propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape);

    /// @brief Уведомляет мир об изменении параметров уже зарегистрированной фигуры.
    /// @param old_id ID фигуры, чьи параметры изменились.
    /// @details Фигура не заменяется и сохраняет свой ID. Уведомление распространяется
    /// по графу владения в обе стороны до тех пор, пока не будут посещены все связанные
    /// фигуры или не сработает ограничение на количество итераций.
    void propose_shape_change(uint64_t old_id);
};

} // namespace godot