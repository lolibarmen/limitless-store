#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <array>
#include <memory>
#include <cstdint>
#include <unordered_map>

namespace godot {

/// @brief Узел октодерева, представляющий пространственный объем (чанк) и хранящий его визуальные данные.
/// Примечание: Экземпляры этого класса управляются классом ChunkOctree.
/// Получение указателя Chunk* по его ID осуществляется через ChunkOctree::get_singleton()->find(id).
class Chunk {
public:
    uint64_t id; ///< Уникальный идентификатор чанка.
    Vector3 center; ///< Центр чанка в мировых координатах.
    float size; ///< Длина стороны чанка.
    int depth; ///< Глубина чанка в октодереве (0 = корневой чанк).

    std::array<std::unique_ptr<Chunk>, 8> children; ///< Дочерние чанки (nullptr, если является листом).
    Chunk* parent = nullptr; ///< Указатель на родительский чанк.

    /// @brief Словарь визуальных представлений фигур в этом чанке (ID фигуры -> MeshInstance3D).
    std::unordered_map<uint64_t, MeshInstance3D*> shape_meshes;

    /// @brief Словарь генераторов материалов для фигур, ожидающих завершения асинхронной генерации меша.
    std::unordered_map<uint64_t, Ref<MaterialGenerator>> shape_material_generators;

    /// @brief Создает новый чанк.
    /// @param id Уникальный идентификатор.
    /// @param c Центр чанка в мировых координатах.
    /// @param s Размер (длина стороны) чанка.
    /// @param d Глубина в октодереве.
    /// @param p Указатель на родительский чанк.
    Chunk(uint64_t id, Vector3 c = Vector3(), float s = 0.0f, int d = 0, Chunk* p = nullptr);

    /// @brief Деструктор. Автоматически очищает дочерние чанки и удаляет связанные MeshInstance3D из сцены.
    ~Chunk();

    // Запрет копирования для предотвращения дублирования указателей на сцену
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    /// @brief Возвращает true, если чанк не имеет дочерних элементов (является листом октодерева).
    bool is_leaf() const { return children[0] == nullptr; }

    /// @brief Вычисляет и возвращает ограничивающий объем (AABB) данного чанка на основе его центра и размера.
    AABB get_aabb() const;

    /// @brief Удаляет визуальное представление (MeshInstance3D) конкретной фигуры из этого чанка и освобождает ресурсы Godot (queue_free).
    /// @param shape_id ID семантической фигуры, меш которой нужно удалить.
    void remove_shape_mesh(uint64_t shape_id);

    /// @brief Полностью очищает все визуальные представления фигур в данном чанке, освобождая ресурсы.
    void clear_all_shape_meshes();
};

} // namespace godot