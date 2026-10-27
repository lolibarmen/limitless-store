#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <cstdint>

namespace godot {

/// @brief Базовый класс генераторов геометрии семантических фигур.
///
/// Задача класса — из описания формы, данного дочерним классом, выполнить отсев
/// треугольников вне чанка, сборку ArrayMesh и доставку результата в WorldMesh.
/// Дочерний класс описывает только форму.
///
/// Источники информации. Дочерний черпает сведения о форме из двух и только двух:
///  - фигура (shape_id) — размеры и свойства сущности;
///  - чанк (chunk_id) — локальная система координат и границы отсечения.
/// Доступ: фигура — SemanticWorld::get_shape(shape_id); чанк —
/// ShapeSpatialManager::get_octree(shape_id)->find(chunk_id).
///
/// Параметры формы. Не хранить в полях класса. Каждый параметр получается одним из:
///  - геттером или публичным полем фигуры либо чанка;
///  - детерминированно из shape_id или chunk_id через Utils/RandomUtils.hpp
///    (randf01 / rand_int; разным параметрам — разные каналы).
///
/// Построение дочернего класса:
///  1. Переопределить generate(uint64_t shape_id, uint64_t chunk_id) const (void).
///  2. MeshBuilder b = builder(shape_id, chunk_id).
///  3. На каждый треугольник формы вызвать b.emit(v0, v1, v2); вершины
///     Vertex{pos, normal, uv} — в локальных координатах чанка (центр = 0).
///  4. Вызвать submit(shape_id, chunk_id, b).
///
/// Запрещено дочернему: хранить параметры формы в полях класса; обращаться к полям
/// MeshBuilder напрямую; вызывать WorldMesh::complete_mesh; резать геометрию по
/// границам чанка; переопределять builder, submit или MeshBuilder::emit.
class MeshGenerator : public Resource {
    GDCLASS(MeshGenerator, Resource)

public:
    /// @brief Вершина треугольника: позиция + нормаль + uv (в локальных координатах чанка).
    struct Vertex {
        Vector3 pos;
        Vector3 normal;
        Vector2 uv;
    };

    /// @brief Аккумулятор геометрии с инкапсулированным моментом остановки.
    /// Поля приватны; единственный способ добавить треугольник — emit(), который сам
    /// классифицирует его против бокса чанка и отбрасывает внеоконные.
    class MeshBuilder {
        friend class MeshGenerator;
        PackedVector3Array verts, norms;
        PackedVector2Array uvs;
        PackedInt32Array   idx;
        PackedInt32Array   marks;   // 1 = треугольник пересекает границу чанка (кандидат на резку)
        AABB               box;     // локальный бокс чанка (центр = 0)
        bool               filter;  // false => бокс недоступен, фильтр выключен (эмитим всё)
    public:
        MeshBuilder() = default;
        /// @brief Эмитит треугольник. Outside-треугольник отбрасывается (не попадает ни в меш, ни в marks).
        void emit(const Vertex& a, const Vertex& b, const Vertex& c);
    };

protected:
    static void _bind_methods();

    /// @brief Создаёт билдер, один раз читая бокс чанка через ShapeSpatialManager.
    /// При недоступности чанка возвращает билдер с выключенным фильтром (геометрия не теряется).
    MeshBuilder builder(uint64_t shape_id, uint64_t chunk_id) const;

    /// @brief Доставка результата. НЕ ПЕРЕОПРЕДЕЛЯТЬ. Строит ArrayMesh из билдера,
    /// (TODO) режет по marks==1, вызывает WorldMesh::complete_mesh(). СЕЙЧАС резка NO-OP.
    void submit(uint64_t shape_id, uint64_t chunk_id, const MeshBuilder& b) const;

public:
    MeshGenerator() = default;
    ~MeshGenerator() = default;

    /// @brief Точка входа. Возвращает void; обязан закончить вызовом submit().
    virtual void generate(uint64_t shape_id, uint64_t chunk_id) const;
};

} // namespace godot