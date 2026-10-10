#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <cstdint>

namespace godot {

class MeshGenerator;

/// @brief Базовый класс для семантических фигур.
/// Управляет графом зависимостей и участвует в системе транзакционных изменений состояния.
class SemanticShape : public Resource {
    GDCLASS(SemanticShape, Resource)

private:
    uint64_t _id = 0;
    godot::PackedInt64Array _owned_shape_ids;

protected:
    AABB _aabb;
    bool _aabb_dirty = true;

    static void _bind_methods();

public:
    SemanticShape();
    virtual ~SemanticShape();

    void set_id(uint64_t id) { _id = id; }
    uint64_t get_id() const { return _id; }

    AABB get_aabb() const;
    
    /// @brief Пересчитывает AABB фигуры при вызове
    virtual void recompute_aabb() { }

    void notify_shape_changed();

    /// @brief Возвращает тип фигуры. Использует StringName для быстрого сравнения O(1).
    virtual StringName get_shape_type() const { return StringName("shape"); }
    
    /// @brief Вычисляет значение SDF (Signed Distance Field) для точки в мировом пространстве.
    virtual float evaluate_sdf(const Vector3& world_pos) const;

    /// @brief Ядро системы LOD. Вызывается при изменении состояния чанка, в котором находится фигура.
    /// @param chunk_id ID изменившегося чанка.
    virtual void on_zone_changed(uint64_t chunk_id) {}

    /// @brief Ядро системы транзакций. Реакция фигуры на изменение зависимого объекта.
    /// @param triggered_old_id ID фигуры, инициировавшей изменение.
    /// @param proposed_shape Предложение (Ref) новой фигуры, в которую переходит triggered_old_id.
    /// @return Ref на новую фигуру для текущей фигуры, или nullptr, если изменений не требуется.
    virtual Ref<SemanticShape> evaluate_reaction(uint64_t triggered_old_id, Ref<SemanticShape> proposed_shape) const {
        return nullptr;
    }

    void add_owned_shape(uint64_t id);
    void remove_owned_shape(uint64_t id);
    bool has_owned_shape(uint64_t id) const;
    godot::PackedInt64Array get_owned_shapes() const;
    void set_owned_shapes(const godot::PackedInt64Array& ids);
    void replace_owned_id(uint64_t old_id, uint64_t new_id);
};

} // namespace godot