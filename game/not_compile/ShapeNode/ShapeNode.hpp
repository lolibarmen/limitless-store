#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <cstdint>

namespace godot {

class WorldMesh;

/// @brief Узел локального октодерева семантической фигуры.
/// Хранит геометрические данные, визуальное состояние (MeshInstance3D) и управляет своим жизненным циклом.
/// В отличие от старого Chunk, ShapeNode не имеет указателей на родителей/детей и не содержит словарей.
/// Идентифицируется через NodeKey (упаковка depth + morton_code).
class ShapeNode {
public:
    uint64_t key; ///< Уникальный ключ узла (NodeKey: depth + morton).
    uint8_t depth; ///< Глубина узла в дереве (дублируется из key для быстрого доступа).
    AABB local_aabb; ///< Ограничивающий объем в локальном пространстве фигуры.
    
    bool geometry_dirty = true; ///< Требует ли узел перегенерации геометрии.
    
    /// @brief Визуальное состояние узла.
    /// Узел сам владеет своим MeshInstance3D и управляет его жизненным циклом.
    MeshInstance3D* mesh_instance = nullptr;
    
    /// @brief Генератор материала, ожидающий завершения асинхронной генерации меша.
    Ref<MaterialGenerator> pending_material;
    
    /// @brief Флаг, indicating что запрос на генерацию меша уже отправлен.
    bool mesh_requested = false;

    /// @brief Конструктор по умолчанию.
    ShapeNode() = default;

    /// @brief Конструктор с параметрами.
    /// @param key Уникальный ключ узла (NodeKey).
    /// @param d Глубина узла.
    /// @param aabb Локальный AABB узла.
    ShapeNode(uint64_t key, uint8_t d, const AABB& aabb);

    /// @brief Деструктор. НЕ удаляет MeshInstance3D автоматически.
    /// Визуальные ресурсы должны быть освобождены через release_mesh() до уничтожения узла.
    ~ShapeNode() = default;

    // Запрет копирования (MeshInstance3D не копируется)
    ShapeNode(const ShapeNode&) = delete;
    ShapeNode& operator=(const ShapeNode&) = delete;

    // Разрешаем перемещение
    ShapeNode(ShapeNode&&) noexcept = default;
    ShapeNode& operator=(ShapeNode&&) noexcept = default;

    /// @brief Запрашивает создание визуального представления (MeshInstance3D) через WorldMesh.
    /// @param wm Указатель на WorldMesh (сервис-контейнер).
    /// @param material_gen Генератор материала для асинхронного применения.
    void request_mesh(WorldMesh* wm, Ref<MaterialGenerator> material_gen);

    /// @brief Освобождает визуальные ресурсы: удаляет MeshInstance3D из сцены.
    /// @param wm Указатель на WorldMesh.
    void release_mesh(WorldMesh* wm);

    /// @brief Применяет готовый меш к существующему MeshInstance3D.
    /// Вызывается после завершения асинхронной генерации.
    /// @param mesh Готовый меш.
    void apply_mesh(Ref<ArrayMesh> mesh);

    /// @brief Применяет материал к существующему MeshInstance3D.
    /// @param material Готовый материал.
    void apply_material(Ref<Material> material);

    /// @brief Проверяет, есть ли у узла активный визуальный меш.
    /// @return true если mesh_instance не nullptr.
    bool has_mesh() const { return mesh_instance != nullptr; }
};

} // namespace godot