#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <cstdint>

namespace godot {

/// @brief Ожидающий запрос отрисовки одной фигуры в одном чанке.
/// Владеет созданными узлами сцены и отбрасывает устаревший результат по токену.
class RenderRequest {
public:
    uint64_t token = 0;
    MeshInstance3D* mesh_instance = nullptr;
    StaticBody3D* collision_body = nullptr;
    CollisionShape3D* collision_shape = nullptr;
    Ref<MeshGenerator> mesh_generator;
    Ref<MaterialGenerator> material_generator;
    bool active = true;

    RenderRequest() = default;
    ~RenderRequest();

    /// @brief Освобождает созданные узлы и сбрасывает ссылку на генератор материала.
    void drop();

    /// @brief Применяет готовый меш и коллизию, если токен всё ещё актуален.
    /// @return false, если запрос устарел (токен не совпал) или уже был применён.
    bool apply(uint64_t shape_id, uint64_t chunk_id, uint64_t p_token,
               const Ref<ArrayMesh>& mesh,
               const Ref<ConcavePolygonShape3D>& collision_faces);
};

} // namespace godot
