#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <cstdint>

namespace godot {

class MeshGenerator;

/// @brief Базовый класс семантических фигур.
///
/// Задача класса — описывать семантическую область: её границы (AABB, SDF), состав
/// (граф подчинённых фигур), систему отсчёта и поведение при изменениях мира (LOD,
/// транзакционные пересечения). Класс не генерирует геометрию и не управляет рендером.
///
/// Параметры формы. Хранятся в полях самой фигуры и служат источником истины для
/// генераторов геометрии (генераторы параметров не хранят, читают их отсюда). Доступ
/// для генераторов — геттеры или публичные поля. При изменении полей-параметров
/// вызывать notify_shape_changed() для инвалидации AABB и уведомления зависимых.
///
/// Построение дочернего класса:
///  1. Объявить поля-параметры формы и открыть к ним доступ геттерами/публичными полями.
///  2. Переопределить get_shape_type() — вернуть StringName типа фигуры.
///  3. Переопределить recompute_aabb() — пересчитать собственный _aabb по полям-параметрам,
///     без побочных эффектов и без мутации других фигур.
///  4. Переопределить evaluate_sdf() — если фигура участвует в пересечениях областей.
///  5. Переопределить on_zone_changed() — если фигура управляет своим LOD/развёртыванием.
///  6. Переопределить evaluate_reaction() — если фигура реагирует на изменения зависимых;
///     возвращать nullptr при отсутствии изменений, не мутировать себя и инициатора
///     (реакция = предложение, применяемое транзакционным слоем).
///  7. Переопределить on_register() — если фигуре нужно собственное октодерево: фигура сама
///     регистрируется в ShapeSpatialManager (create_octree / attach_shape) или связывает себя
///     с сеткой родителя (bind_spatially).
///  8. Состав фигуры задавать через add_owned_shape / remove_owned_shape / replace_owned_id.
///
/// Запрещено дочернему: генерировать геометрию и обращаться к MeshGenerator / WorldMesh;
/// менять _aabb напрямую (только recompute_aabb + get_aabb); из evaluate_reaction и
/// on_zone_changed мутировать чужие фигуры или их owned-графы.
class SemanticShape : public Resource {
    GDCLASS(SemanticShape, Resource)

private:

protected:
    uint64_t _id = 0;
    godot::PackedInt64Array _owned_shape_ids;

    AABB _aabb;
    bool _aabb_dirty = true;

    static void _bind_methods();
    
public:
    SemanticShape();
    virtual ~SemanticShape();

    void set_id(uint64_t id) { _id = id; }
    uint64_t get_id() const { return _id; }

    AABB get_aabb();
    
    /// @brief Пересчитывает AABB фигуры при вызове.
    virtual void recompute_aabb() { }

    void notify_shape_changed();

    /// @brief Возвращает тип фигуры. Использует StringName для быстрого сравнения O(1).
    virtual StringName get_shape_type() const { return StringName("shape"); }
    
    /// @brief Вычисляет значение SDF (Signed Distance Field) для точки в мировом пространстве.
    virtual float evaluate_sdf(const Vector3& world_pos) const;

    /// @brief Ядро системы LOD. Вызывается при активации чанка в октодереве, к которому привязана фигура.
    /// @param chunk_id ID активированного чанка.
    /// @details Привязку выполняет сама фигура при регистрации (см. on_register()):
    ///  - фигура с собственным октодеревом вызывает create_octree() в ShapeSpatialManager;
    ///  - фигура без собственного дерева связывается с сеткой родителя через bind_spatially().
    ///
    /// Фигура не хранит ссылку на октодерево.
    ///
    /// Если фигура логически мала, но может пересекать несколько чанков, обработчик обязан
    /// подавлять дублирование: выполнять эффект только когда центр AABB фигуры лежит внутри chunk_id.
    virtual void on_zone_changed(uint64_t chunk_id) {}

    /// @brief Хук регистрации фигуры в SemanticWorld. Вызывается сразу после присвоения ID.
    /// @details По умолчанию ничего не делает: такая фигура не участвует в пространственном
    /// разбиении до тех пор, пока явно не зарегистрируется в ShapeSpatialManager.
    /// Фигурам, которым нужно собственное октодерево, следует переопределить этот метод и
    /// вызвать в нём ShapeSpatialManager::create_octree(get_id(), get_aabb(), ...) с
    /// необходимыми параметрами (дистанции перехода на более глубокий уровень, размер корня).
    /// Дочерние фигуры,
    /// использующие сетку родителя, могут вызвать bind_spatially(get_id(), parent_id).
    virtual void on_register() {}

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