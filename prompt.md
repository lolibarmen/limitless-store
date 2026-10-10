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
```
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
```

***

Пример фигуры:
```
#pragma once

#include <SemanticSphere/SemanticSphere.hpp>
#include <CubeMeshGenerator/CubeMeshGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

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

    void set_pos(Vector3 p_pos) { _pos = p_pos; }
    Vector3 get_pos() const { return _pos; }

    void set_size(Vector3 p_size) { _size = p_size; }
    Vector3 get_size() const { return _size; }
};

} // namespace godot
```
```
#include "TestCube.hpp"
#include <WorldMesh/WorldMesh.hpp>
#include <ChunkOctree/ChunkOctree.hpp>
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

using namespace godot;

void TestCube::_bind_methods() {

}

TestCube::TestCube() {
    _pos = Vector3(-10.0, -10.0, -10.0);
    _size = Vector3( 20.0,  20.0,  20.0);

    _mesh_generator.instantiate();
    _mesh_generator->set_pos(_pos);
    _mesh_generator->set_size(_size);

    _material_generator.instantiate();
    _material_generator->set_albedo_texture(ResourceLoader::get_singleton()->load("res://assets/test_texture.png"));
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
```
```
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
```
```
#include "CubeMeshGenerator.hpp"
#include <ChunkOctree/ChunkOctree.hpp>
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

    Chunk* chunk = ChunkOctree::get_singleton()->find(chunk_id);
    if (!chunk) return mesh;

    AABB chunk_aabb = chunk->get_aabb();
    
    Vector3 c_min = pos;
    Vector3 c_max = pos + size;
    
    Vector3 ch_min = chunk_aabb.position;
    Vector3 ch_max = chunk_aabb.position + chunk_aabb.size;

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

    float x0, x1, y0, y1, z0, z1;

    if (c_max.x >= ch_min.x && c_max.x <= ch_max.x) {
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (y0 < y1 && z0 < z1) 
            add_quad(Vector3(c_max.x, y0, z0), Vector3(c_max.x, y1, z0), Vector3(c_max.x, y1, z1), Vector3(c_max.x, y0, z1), Vector3(1, 0, 0));
    }

    if (c_min.x >= ch_min.x && c_min.x <= ch_max.x) {
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (y0 < y1 && z0 < z1) 
            add_quad(Vector3(c_min.x, y0, z1), Vector3(c_min.x, y1, z1), Vector3(c_min.x, y1, z0), Vector3(c_min.x, y0, z0), Vector3(-1, 0, 0));
    }

    if (c_max.y >= ch_min.y && c_max.y <= ch_max.y) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (x0 < x1 && z0 < z1) 
            add_quad(Vector3(x0, c_max.y, z1), Vector3(x1, c_max.y, z1), Vector3(x1, c_max.y, z0), Vector3(x0, c_max.y, z0), Vector3(0, 1, 0));
    }

    if (c_min.y >= ch_min.y && c_min.y <= ch_max.y) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        z0 = MAX(c_min.z, ch_min.z); z1 = MIN(c_max.z, ch_max.z);
        if (x0 < x1 && z0 < z1) 
            add_quad(Vector3(x0, c_min.y, z0), Vector3(x1, c_min.y, z0), Vector3(x1, c_min.y, z1), Vector3(x0, c_min.y, z1), Vector3(0, -1, 0));
    }

    if (c_max.z >= ch_min.z && c_max.z <= ch_max.z) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        if (x0 < x1 && y0 < y1) 
            add_quad(Vector3(x0, y0, c_max.z), Vector3(x1, y0, c_max.z), Vector3(x1, y1, c_max.z), Vector3(x0, y1, c_max.z), Vector3(0, 0, 1));
    }

    if (c_min.z >= ch_min.z && c_min.z <= ch_max.z) {
        x0 = MAX(c_min.x, ch_min.x); x1 = MIN(c_max.x, ch_max.x);
        y0 = MAX(c_min.y, ch_min.y); y1 = MIN(c_max.y, ch_max.y);
        if (x0 < x1 && y0 < y1) 
            add_quad(Vector3(x1, y0, c_min.z), Vector3(x0, y0, c_min.z), Vector3(x0, y1, c_min.z), Vector3(x1, y1, c_min.z), Vector3(0, 0, -1));
    }

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
```