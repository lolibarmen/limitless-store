#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <cstdint>
#include <deque>
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

    struct PendingRender {
        uint64_t shape_id = 0;
        uint64_t chunk_id = 0;
        uint64_t token = 0;
    };

    struct ActiveRender {
        uint64_t shape_id = 0;
        uint64_t chunk_id = 0;
    };

    /// @brief Очередь задач, ожидающих свободного слота генерации.
    std::deque<PendingRender> _pending_renders;

    /// @brief Активные задачи: token -> (shape_id, chunk_id). Задача занимает слот до complete_mesh/finish_render.
    std::unordered_map<uint64_t, ActiveRender> _active_renders;

    /// @brief Актуальные токены ожидающих задач: shape_id -> (chunk_id -> token).
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> _pending_tokens;

    /// @brief Максимум одновременно выполняющихся генераций меша.
    int _max_concurrent_renders = 0;
    bool _pumping_renders = false;

    /// @brief Запускает задачи из очереди, пока есть свободные слоты.
    void _pump_renders();

    /// @brief Создаёт сценовые узлы и запускает генерацию для задачи, если она всё ещё актуальна.
    bool _start_render(const PendingRender& job);

    /// @brief Убирает пару из очереди ожидания и инвалидирует её токен.
    void _cancel_pending(uint64_t shape_id, uint64_t chunk_id);

    /// @brief Освобождает слот активной задачи по токену.
    void _release_active(uint64_t token);

    /// @brief Deferred-завершение задачи без результата (вызывается из рабочих потоков).
    void _on_render_task_finished(uint64_t shape_id, uint64_t chunk_id, uint64_t token);

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

    /// @brief Максимум одновременно выполняющихся генераций меша.
    int get_max_concurrent_renders() const { return _max_concurrent_renders; }
    void set_max_concurrent_renders(int count);

    /// @brief Инициирует процесс рендеринга фигуры в заданном чанке.
    /// Очищает старые меши этой фигуры в текущем чанке и его родителях для предотвращения визуальных артефактов.
    /// Задача ставится в очередь и запускается только при наличии свободного слота;
    /// сценовые узлы создаются в момент старта задачи.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, в котором требуется отрисовка.
    /// @param mesh_generator Генератор геометрии.
    /// @param material_generator Генератор материалов.
    void request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> mesh_generator, Ref<MaterialGenerator> material_generator);

    /// @brief Отменяет запрос на рендеринг и удаляет визуальное представление фигуры из чанка.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, из которого нужно удалить меш.
    void cancel_render(uint64_t shape_id, uint64_t chunk_id);

    /// @brief Инвалидирует запрос пары (фигура, чанк) без удаления узлов.
    /// Используется при уничтожении чанка/запроса: активная задача доживёт и сама освободит слот.
    void cancel_request(uint64_t shape_id, uint64_t chunk_id);

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
