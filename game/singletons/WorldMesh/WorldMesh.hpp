#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace godot {

class MeshGenerator;
class MaterialGenerator;
class Chunk;

/// @brief Управляет визуальным и коллизионным представлением семантических фигур в октодереве чанков.
/// Отвечает за создание, обновление и очистку MeshInstance3D, StaticBody3D и CollisionShape3D,
/// предотвращая артефакты LOD и устаревшие коллизии.
class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)

private:
    /// @brief Токены актуальности запросов: shape_id -> (chunk_id -> token).
    mutable std::mutex _tokens_mutex;
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> _tokens;
    uint64_t _next_token = 1;

    /// @brief Фигуры, ожидающие перерисовки после изменения параметров.
    std::unordered_set<uint64_t> _dirty_shapes;
    double _dirty_timer = 0.0;

    /// @brief Удаляет визуальное и коллизионное представление фигуры из конкретного чанка.
    void _remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id);

    /// @brief Рекурсивно удаляет меш фигуры из чанка и всех его дочерних элементов.
    /// Используется для корректного перехода LOD, чтобы дочерние чанки не рендерили то, что уже рендерит родитель.
    void _clear_shape_in_subtree(Chunk* node, uint64_t shape_id);

    /// @brief Удаляет запросы всех ожидающих фигур и помечает их для повторной отрисовки.
    void _flush_invalidations();

protected:
    static void _bind_methods();

public:
    /// @brief Возвращает глобальный экземпляр WorldMesh, зарегистрированный в Godot Engine.
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    /// @brief Инициирует процесс рендеринга фигуры в заданном чанке.
    /// Очищает старые меши этой фигуры в текущем чанке и его родителях для предотвращения визуальных артефактов.
    /// Создает базовые MeshInstance3D, StaticBody3D и CollisionShape3D, сохраняет генераторы
    /// для последующего применения материалов и геометрии коллизии.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, в котором требуется отрисовка.
    /// @param mesh_generator Генератор геометрии.
    /// @param material_generator Генератор материалов.
    void request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> mesh_generator, Ref<MaterialGenerator> material_generator);

    /// @brief Отменяет запрос на рендеринг и удаляет визуальное представление фигуры из чанка.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, из которого нужно удалить меш.
    void cancel_render(uint64_t shape_id, uint64_t chunk_id);

    /// @brief Завершает асинхронный процесс рендеринга, применяя сгенерированный Mesh, Material и коллизию.
    /// Устаревшая версия без токена: использует токен текущего запроса чанка.
    void complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh, Ref<ConcavePolygonShape3D> collision_shape = Ref<ConcavePolygonShape3D>());

    /// @brief Завершает асинхронный процесс рендеринга, применяя результат только если токен актуален.
    /// @param token Токен, полученный при постановке задачи генерации.
    void complete_mesh(uint64_t shape_id, uint64_t chunk_id, uint64_t token, Ref<ArrayMesh> mesh, Ref<ConcavePolygonShape3D> collision_shape = Ref<ConcavePolygonShape3D>());

    /// @brief Возвращает текущий токен запроса для пары (фигура, чанк) или 0.
    uint64_t current_token(uint64_t shape_id, uint64_t chunk_id) const;

    /// @brief Проверяет, что токен всё ещё актуален для пары (фигура, чанк).
    bool is_current(uint64_t shape_id, uint64_t chunk_id, uint64_t token) const;

    /// @brief Помечает фигуру изменённой: её запросы будут удалены и перерисованы после короткой задержки.
    void invalidate_shape(uint64_t shape_id);

    /// @brief Немедленно удаляет все запросы и узлы фигуры без перерисовки.
    void drop_shape(uint64_t shape_id);
};

} // namespace godot
