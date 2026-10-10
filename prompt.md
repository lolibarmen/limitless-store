```

#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <cstdint>

namespace godot {

/// @brief Глобальный менеджер семантических фигур.
/// Управляет их жизненным циклом, графом зависимостей и транзакционными изменениями состояния.
class SemanticWorld : public Object {
    GDCLASS(SemanticWorld, Object)

private:
    std::unordered_map<uint64_t, Ref<SemanticShape>> _shapes;
    Ref<Mutex> _shapes_mutex;
    std::atomic<uint64_t> _next_id{1};

protected:
    static void _bind_methods();

public:
    SemanticWorld();
    ~SemanticWorld() override = default;

    /// @brief Возвращает глобальный экземпляр SemanticWorld, зарегистрированный в Godot Engine.
    static SemanticWorld* get_singleton();

    /// @brief Регистрирует новую фигуру в мире, присваивает ей уникальный ID и возвращает его.
    /// @param shape Ссылка на фигуру (должна быть валидной).
    /// @return Уникальный ID зарегистрированной фигуры. Поточно-безопасен.
    uint64_t register_shape(Ref<SemanticShape> shape);

    /// @brief Удаляет фигуру из мира по её ID. Поточно-безопасен.
    /// @param id Уникальный ID удаляемой фигуры.
    void unregister_shape(uint64_t id);

    /// @brief Возвращает ссылку на фигуру по её ID.
    /// @param id Уникальный ID фигуры.
    /// @return Ref на фигуру. Возвращает невалидный Ref, если фигура не найдена. Поточно-безопасен.
    Ref<SemanticShape> get_shape(uint64_t id) const;

    /// @brief Уведомляет фигуры об изменении состояния чанков (зон).
    /// Используется системой LOD для обновления состояния фигур при загрузке/выгрузке областей.
    /// @param chunk_ids Вектор ID чанков, состояние которых изменилось.
    void on_zone_changed(const std::vector<uint64_t>& chunk_ids);

    /// @brief Инициирует транзакционное изменение состояния фигуры.
    /// Запускает каскадную оценку реакций (evaluate_reaction) у зависимых объектов и атомарно 
    /// применяет все одобренные изменения, обновляя граф зависимостей.
    /// @param old_id ID фигуры, которая должна быть заменена.
    /// @param proposed_shape Предложение (Ref) новой, еще не зарегистрированной фигуры.
    void propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape);
};

} // namespace godot

```

```
#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <cstdint>

namespace godot {

class MeshGenerator;
class MaterialGenerator;
class Chunk;

/// @brief Управляет визуальным представлением (меши и материалы) семантических фигур в октодереве чанков.
/// Отвечает за создание, обновление и очистку MeshInstance3D, предотвращая артефакты LOD.
class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)
    
private:
    /// @brief Удаляет визуальное представление фигуры из конкретного чанка.
    void _remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id);
    
    /// @brief Рекурсивно удаляет меш фигуры из чанка и всех его дочерних элементов.
    /// Используется для корректного перехода LOD, чтобы дочерние чанки не рендерили то, что уже рендерит родитель.
    void _clear_shape_in_subtree(Chunk* node, uint64_t shape_id);

protected:
    static void _bind_methods();

public:
    /// @brief Возвращает глобальный экземпляр WorldMesh, зарегистрированный в Godot Engine.
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    /// @brief Инициирует процесс рендеринга фигуры в заданном чанке.
    /// Очищает старые меши этой фигуры в текущем чанке и его родителях для предотвращения визуальных артефактов.
    /// Создает базовый MeshInstance3D и сохраняет генераторы для последующего применения материалов.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, в котором требуется отрисовка.
    /// @param mesh_generator Генератор геометрии.
    /// @param material_generator Генератор материалов.
    void request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> mesh_generator, Ref<MaterialGenerator> material_generator);
    
    /// @brief Отменяет запрос на рендеринг и удаляет визуальное представление фигуры из чанка.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, из которого нужно удалить меш.
    void cancel_render(uint64_t shape_id, uint64_t chunk_id);
    
    /// @brief Завершает асинхронный процесс рендеринга, применяя сгенерированный Mesh и Material.
    /// Вызывается (обычно из главного потока), когда тяжелая генерация меша завершена.
    /// @param shape_id Уникальный ID семантической фигуры.
    /// @param chunk_id ID чанка, к которому применяется меш.
    /// @param mesh Готовый к использованию Ref<ArrayMesh>.
    void complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh);
};

} // namespace godot
```

```
#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <array>
#include <vector>
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
```

```
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
```