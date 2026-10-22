#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <unordered_map>
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

    /// @brief Уведомляет конкретную фигуру об изменении состояния её чанков (зон).
    /// @param shape_id ID фигуры, чьи чанки изменились.
    /// @param chunk_ids Вектор ID добавленных чанков.
    void on_zone_changed(uint64_t shape_id, const std::vector<uint64_t>& chunk_ids);

    /// @brief Инициирует транзакционное изменение состояния фигуры.
    /// Запускает каскадную оценку реакций (evaluate_reaction) у зависимых объектов и атомарно
    /// применяет все одобренные изменения, обновляя граф зависимостей.
    /// @param old_id ID фигуры, которая должна быть заменена.
    /// @param proposed_shape Предложение (Ref) новой, еще не зарегистрированной фигуры.
    void propose_shape_change(uint64_t old_id, Ref<SemanticShape> proposed_shape);
};

} // namespace godot

#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
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

#pragma once

#include "ChunkOctree.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <atomic>
#include <cstdint>

namespace godot {

/// @brief Глобальный менеджер пространственного разбиения.
/// Управляет созданием ChunkOctree для фигур и обеспечивает наследование пространственной сетки.
class ShapeSpatialManager : public Object {
    GDCLASS(ShapeSpatialManager, Object)

private:
    /// Деревья, которыми этот менеджер владеет (созданы для is_relative == true или корневых фигур)
    std::unordered_map<uint64_t, std::unique_ptr<ChunkOctree>> _owned_octrees;
    
    /// Пространственная привязка: для каждой фигуры указывает, чье дерево она использует.
    /// Может указывать на собственное дерево или на дерево родителя.
    std::unordered_map<uint64_t, ChunkOctree*> _shape_to_octree;
    
    std::atomic<uint64_t> _next_chunk_id{1};

protected:
    static void _bind_methods() {}

public:
    static ShapeSpatialManager* get_singleton();

    /// @brief Регистрирует фигуру и автоматически решает, нужно ли ей собственное октодерево.
    void register_shape(uint64_t shape_id, Ref<SemanticShape> shape);

    /// @brief Связывает дочернюю фигуру с пространственной сеткой родительской.
    void bind_spatially(uint64_t child_shape_id, uint64_t parent_shape_id);

    /// @brief Удаляет фигуру и корректно очищает ссылки на её дерево у дочерних элементов.
    void unregister_shape(uint64_t shape_id);

    /// @brief Возвращает октодерево, которое должна использовать данная фигура.
    ChunkOctree* get_octree(uint64_t shape_id) const;

    /// @brief Итерация только по тем фигурам, которые владеют собственным октодеревом.
    void for_each_owned_octree(std::function<void(uint64_t shape_id, ChunkOctree* octree)> func);

    /// @brief Глобальный провайдер уникальных ID для всех чанков в сцене.
    uint64_t get_next_chunk_id() { return _next_chunk_id++; }
};

} // namespace godot

#pragma once
#include "Chunk.hpp"
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <cstdint>

namespace godot {

class ChunkOctree {
    
private:
    std::unordered_map<uint64_t, Chunk*> roots;
    std::unordered_set<Chunk*> leaf_chunks;
    std::unordered_map<uint64_t, Chunk*> id_to_chunk;

    void destroy_subtree(Chunk* node);

    /// @brief Рекурсивно оценивает и обновляет LOD для конкретного узла и его потомков.
    void evaluate_node(Chunk* node, const Vector3& viewer_pos, int max_depth);
    
public:
    ~ChunkOctree() { clear(); }

    uint64_t add_root(const Vector3& center, float size);
    void remove_root(uint64_t id);
    
    void for_each_root(std::function<void(Chunk*)> func);

    void split(Chunk* node);
    void collapse(Chunk* node);
    
    /// @brief Автоматически оценивает и обновляет LOD дерева на основе позиции наблюдателя.
    void evaluate_and_update_lod(const Vector3& viewer_pos, int max_depth);

    void for_each_leaf(std::function<void(Chunk*)> func);

    Chunk* find(uint64_t id) const;

    void clear();
};

} // namespace godot

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

    /// @brief Словарь визуальных представлений фигур в этом чанке (ID фигуры -> MeshInstance3D).
    std::unordered_map<uint64_t, MeshInstance3D*> shape_meshes;

    /// @brief Словарь генераторов материалов для фигур, ожидающих завершения асинхронной генерации меша.
    std::unordered_map<uint64_t, Ref<MaterialGenerator>> shape_material_generators;

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

    /// @brief Удаляет визуальное представление (MeshInstance3D) конкретной фигуры из этого чанка и освобождает ресурсы Godot (queue_free).
    /// @param shape_id ID семантической фигуры, меш которой нужно удалить.
    void remove_shape_mesh(uint64_t shape_id);

    /// @brief Полностью очищает все визуальные представления фигур в данном чанке, освобождая ресурсы.
    void clear_all_shape_meshes();
};

} // namespace godot

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

protected:
    uint64_t _id = 0;
    godot::PackedInt64Array _owned_shape_ids;
    bool _is_relative = false; ///< Движется ли фигура относительно своего пространственного родителя.
    
    AABB _aabb;
    bool _aabb_dirty = true;

    static void _bind_methods();

public:
    SemanticShape();
    virtual ~SemanticShape();

    void set_id(uint64_t id) { _id = id; }
    uint64_t get_id() const { return _id; }
    
    /// @brief Возвращает true, если фигура имеет собственное независимое пространство 
    /// (движется относительно родителя, имеет свою физику и т.д.).
    virtual bool is_relative() const { return _is_relative; }
    void set_relative(bool value) { _is_relative = value; }

    AABB get_aabb();
    
    /// @brief Пересчитывает AABB фигуры при вызове.
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

Пример фигуры:
#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <CubeMeshGenerator/CubeMeshGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Тестовая семантическая фигура в виде куба.
/// Демонстрирует интеграцию с новой архитектурой локальных октодеревьев.
/// Управляет собственным LOD через переопределённый sync_lod.
class TestCube : public SemanticShape {
    GDCLASS(TestCube, SemanticShape)

private:
    Ref<CubeMeshGenerator> _mesh_generator;
    Ref<TriplanarMaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

    Vector3 _pos = {};
    Vector3 _size = {};

public:
    TestCube();
    virtual ~TestCube() {};

    StringName get_shape_type() const override { return StringName("test_cube"); }

    void recompute_aabb();

    virtual void on_zone_changed(uint64_t chunk_id) override;

    void set_pos(Vector3 p_pos) { _pos = p_pos; notify_shape_changed(); }
    Vector3 get_pos() const { return _pos; }

    void set_size(Vector3 p_size) { _size = p_size; notify_shape_changed(); }
    Vector3 get_size() const { return _size; }
};

} // namespace godot

#include "TestCube.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void TestCube::_bind_methods() {
    // Binding'и для Godot, если нужны
}

TestCube::TestCube() {
    _is_relative = true;
    
    _pos = Vector3(-10.0, -10.0, -10.0);
    _size = Vector3(20.0, 20.0, 20.0);

    _mesh_generator.instantiate();
    _mesh_generator->set_pos(_pos);
    _mesh_generator->set_size(_size);

    _material_generator.instantiate();
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load("res://assets/test_texture.jpg"));
}

void TestCube::recompute_aabb() {
    _aabb = AABB(_pos, _size);
}

void TestCube::on_zone_changed(uint64_t chunk_id) {
    WorldMesh* wm = WorldMesh::get_singleton();
    if (!wm) {
        return;
    }

    if (_mesh_generator.is_null() || _material_generator.is_null()) {
        return;
    }

    wm->request_render(get_id(), chunk_id, _mesh_generator, _material_generator);
}

#pragma once

#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

class CubeMeshGenerator : public MeshGenerator {
    GDCLASS(CubeMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

private:
    Vector3 pos = {0.0, 0.0, 0.0};
    Vector3 size = {0.0, 0.0, 0.0};

public:
    CubeMeshGenerator();
    ~CubeMeshGenerator();

    void set_pos(Vector3 p_pos) { pos = p_pos; }
    Vector3 get_pos() const { return pos; }

    void set_size(Vector3 p_size) { size = p_size; }
    Vector3 get_size() const { return size; }

    virtual Ref<ArrayMesh> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot

#include "CubeMeshGenerator.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp> // <-- ДОБАВЛЕНО
#include <SemanticWorld/SemanticWorld.hpp>             // <-- ДОБАВЛЕНО
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

CubeMeshGenerator::CubeMeshGenerator() = default;
CubeMeshGenerator::~CubeMeshGenerator() = default;

void CubeMeshGenerator::_bind_methods() { }

Ref<ArrayMesh> CubeMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<ArrayMesh> mesh;
    mesh.instantiate();

    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return mesh;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return mesh;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return mesh;

    auto* world = SemanticWorld::get_singleton();
    Ref<SemanticShape> shape = world->get_shape(shape_id);
    if (!shape.is_valid()) return mesh;

    // 1. Получаем AABB в мировых координатах
    AABB shape_aabb = shape->get_aabb();
    AABB chunk_aabb = chunk->get_aabb();

    // 2. Находим реальное пересечение фигуры и чанка
    AABB intersect = shape_aabb.intersection(chunk_aabb);
    
    // Если пересечения нет (или оно вырождено), меш не нужен
    if (intersect.size.x <= 0.001f || intersect.size.y <= 0.001f || intersect.size.z <= 0.001f) {
        return mesh;
    }

    // 3. Переводим координаты пересечения в локальное пространство чанка (относительно его центра)
    Vector3 center = chunk_aabb.get_center();
    Vector3 local_min = intersect.position - center;
    Vector3 local_max = intersect.position + intersect.size - center;

    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    auto add_quad = [&](Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3, Vector3 normal) {
        int base = vertices.size();
        vertices.push_back(v0); vertices.push_back(v1);
        vertices.push_back(v2); vertices.push_back(v3);
        
        for (int i = 0; i < 4; i++) normals.push_back(normal);
        
        uvs.push_back(Vector2(0, 1)); uvs.push_back(Vector2(0, 0));
        uvs.push_back(Vector2(1, 0)); uvs.push_back(Vector2(1, 1));
        
        indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base); indices.push_back(base + 3); indices.push_back(base + 2);
    };

    // Генерируем 6 граней строго для области пересечения
    // Правая (X+)
    add_quad(Vector3(local_max.x, local_min.y, local_min.z), Vector3(local_max.x, local_max.y, local_min.z), Vector3(local_max.x, local_max.y, local_max.z), Vector3(local_max.x, local_min.y, local_max.z), Vector3(1, 0, 0));
    // Левая (X-)
    add_quad(Vector3(local_min.x, local_min.y, local_max.z), Vector3(local_min.x, local_max.y, local_max.z), Vector3(local_min.x, local_max.y, local_min.z), Vector3(local_min.x, local_min.y, local_min.z), Vector3(-1, 0, 0));
    // Верхняя (Y+)
    add_quad(Vector3(local_min.x, local_max.y, local_max.z), Vector3(local_max.x, local_max.y, local_max.z), Vector3(local_max.x, local_max.y, local_min.z), Vector3(local_min.x, local_max.y, local_min.z), Vector3(0, 1, 0));
    // Нижняя (Y-)
    add_quad(Vector3(local_min.x, local_min.y, local_min.z), Vector3(local_max.x, local_min.y, local_min.z), Vector3(local_max.x, local_min.y, local_max.z), Vector3(local_min.x, local_min.y, local_max.z), Vector3(0, -1, 0));
    // Передняя (Z+)
    add_quad(Vector3(local_min.x, local_min.y, local_max.z), Vector3(local_max.x, local_min.y, local_max.z), Vector3(local_max.x, local_max.y, local_max.z), Vector3(local_min.x, local_max.y, local_max.z), Vector3(0, 0, 1));
    // Задняя (Z-)
    add_quad(Vector3(local_max.x, local_min.y, local_min.z), Vector3(local_min.x, local_min.y, local_min.z), Vector3(local_min.x, local_max.y, local_min.z), Vector3(local_max.x, local_max.y, local_min.z), Vector3(0, 0, -1));

    if (vertices.is_empty()) return mesh;

    Array mesh_array;
    mesh_array.resize(Mesh::ARRAY_MAX);
    mesh_array[Mesh::ARRAY_VERTEX] = vertices;
    mesh_array[Mesh::ARRAY_NORMAL] = normals;
    mesh_array[Mesh::ARRAY_TEX_UV] = uvs;
    mesh_array[Mesh::ARRAY_INDEX] = indices;

    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
    return mesh;
}