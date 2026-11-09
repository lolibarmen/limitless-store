#pragma once

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <array>
#include <memory>
#include <cstdint>
#include <unordered_map>

namespace godot {

class RenderRequest;

/// @brief Узел октодерева, представляющий пространственный объем (чанк) и хранящий его визуальные данные.
/// Примечание: Экземпляры этого класса управляются экземпляром ChunkOctree.
/// Получение указателя Chunk* по его ID осуществляется через ShapeSpatialManager::get_singleton()->get_octree(shape_id)->find(id).
class Chunk {
public:
    uint64_t id; ///< Уникальный глобальный идентификатор чанка.
    Vector3 center; ///< Центр чанка в мировых координатах.
    float size; ///< Длина стороны чанка.
    int depth; ///< Глубина чанка в октодереве (0 = корневой чанк).

    std::array<std::unique_ptr<Chunk>, 8> children; ///< Дочерние чанки (nullptr, если является листом).
    Chunk* parent = nullptr; ///< Указатель на родительский чанк.

    /// @brief Словарь ожидающих запросов отрисовки фигур в этом чанке (ID фигуры -> запрос).
    /// Запрос владеет созданными узлами и отбрасывает устаревшие асинхронные результаты по токену.
    std::unordered_map<uint64_t, std::unique_ptr<RenderRequest>> shape_requests;

    /// @brief Создает новый чанк.
    /// @param id Уникальный глобальный идентификатор.
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

    /// @brief Удаляет и визуальное, и коллизионное представление фигуры из этого чанка.
    /// @param shape_id ID семантической фигуры.
    void remove_shape(uint64_t shape_id);

    /// @brief Полностью очищает все визуальные и коллизионные представления фигур в данном чанке, освобождая ресурсы.
    void clear_all_shape_meshes();
};

} // namespace godot