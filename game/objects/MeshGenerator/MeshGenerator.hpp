#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>

namespace godot {

class SemanticShape;

/// @brief Базовый класс для генерации геометрии (ArrayMesh) семантических фигур.
class MeshGenerator : public Resource {
    GDCLASS(MeshGenerator, Resource)

protected:
    static void _bind_methods();

public:
    MeshGenerator() = default;
    ~MeshGenerator() = default;

    /// @brief Генерирует меш для указанной фигуры в заданном чанке.
    /// Поддерживает два режима работы:
    /// 1. Синхронный: для простых фигур генерирует и возвращает Ref<ArrayMesh> немедленно в главном потоке.
    /// 2. Асинхронный: для сложных вычислений возвращает nullptr, запускает фоновый поток, 
    ///    который по завершении должен самостоятельно вызвать WorldMesh::complete_mesh().
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, для которого производится генерация.
    /// @return Ref<ArrayMesh> с готовой геометрией (при синхронной генерации) или nullptr (при асинхронной).
    virtual Ref<ArrayMesh> generate(uint64_t shape_id, uint64_t chunk_id) const;
};

} // namespace godot