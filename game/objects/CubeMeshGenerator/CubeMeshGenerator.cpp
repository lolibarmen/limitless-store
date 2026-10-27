#include "CubeMeshGenerator.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void CubeMeshGenerator::_bind_methods() {
    // Параметры формы больше не хранятся в полях класса, поэтому привязка методов отсутствует.
    // Все данные извлекаются динамически через shape_id и chunk_id.
}

void CubeMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    auto* mgr = ShapeSpatialManager::get_singleton();
    if (!mgr) return;

    ChunkOctree* octree = mgr->get_octree(shape_id);
    if (!octree) return;

    Chunk* chunk = octree->find(chunk_id);
    if (!chunk) return;

    auto* world = SemanticWorld::get_singleton();
    Ref<SemanticShape> shape = world->get_shape(shape_id);
    if (!shape.is_valid()) return;

    // 1. Получаем AABB фигуры в мировых координатах (источник истины о форме)
    AABB shape_aabb = shape->get_aabb();
    
    // Проверка на вырожденную фигуру
    if (shape_aabb.size.x <= 0.0f || shape_aabb.size.y <= 0.0f || shape_aabb.size.z <= 0.0f) {
        return;
    }

    // 2. Получаем центр чанка для перевода координат в локальное пространство (центр = 0)
    AABB chunk_aabb = chunk->get_aabb();
    Vector3 chunk_center = chunk_aabb.get_center();

    // 3. Переводим координаты фигуры в локальное пространство чанка
    Vector3 local_min = shape_aabb.position - chunk_center;
    Vector3 local_max = shape_aabb.position + shape_aabb.size - chunk_center;

    // 4. Создаем билдер (он сам инициализирует локальный бокс чанка для фильтрации)
    MeshBuilder b = builder(shape_id, chunk_id);

    // 5. Вспомогательная лямбда для эмита квада (2 треугольника) с корректными UV и нормалями
    auto emit_quad = [&](const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& normal) {
        Vertex vert0 = {v0, normal, Vector2(0, 1)};
        Vertex vert1 = {v1, normal, Vector2(0, 0)};
        Vertex vert2 = {v2, normal, Vector2(1, 0)};
        Vertex vert3 = {v3, normal, Vector2(1, 1)};
        
        // Порядок вершин сохранён как в оригинальном генераторе для корректного winding order
        b.emit(vert0, vert2, vert1);
        b.emit(vert0, vert3, vert2);
    };

    // 6. Генерируем 6 граней куба в локальных координатах чанка.
    // MeshBuilder::emit самостоятельно отбросит треугольники, полностью находящиеся вне чанка,
    // и пометит те, что пересекают границу (резка будет выполнена на этапе submit).
    
    // Правая (X+)
    emit_quad(
        Vector3(local_max.x, local_min.y, local_min.z),
        Vector3(local_max.x, local_max.y, local_min.z),
        Vector3(local_max.x, local_max.y, local_max.z),
        Vector3(local_max.x, local_min.y, local_max.z),
        Vector3(1, 0, 0)
    );
    
    // Левая (X-)
    emit_quad(
        Vector3(local_min.x, local_min.y, local_max.z),
        Vector3(local_min.x, local_max.y, local_max.z),
        Vector3(local_min.x, local_max.y, local_min.z),
        Vector3(local_min.x, local_min.y, local_min.z),
        Vector3(-1, 0, 0)
    );
    
    // Верхняя (Y+)
    emit_quad(
        Vector3(local_min.x, local_max.y, local_max.z),
        Vector3(local_max.x, local_max.y, local_max.z),
        Vector3(local_max.x, local_max.y, local_min.z),
        Vector3(local_min.x, local_max.y, local_min.z),
        Vector3(0, 1, 0)
    );
    
    // Нижняя (Y-)
    emit_quad(
        Vector3(local_min.x, local_min.y, local_min.z),
        Vector3(local_max.x, local_min.y, local_min.z),
        Vector3(local_max.x, local_min.y, local_max.z),
        Vector3(local_min.x, local_min.y, local_max.z),
        Vector3(0, -1, 0)
    );
    
    // Передняя (Z+)
    emit_quad(
        Vector3(local_min.x, local_min.y, local_max.z),
        Vector3(local_max.x, local_min.y, local_max.z),
        Vector3(local_max.x, local_max.y, local_max.z),
        Vector3(local_min.x, local_max.y, local_max.z),
        Vector3(0, 0, 1)
    );
    
    // Задняя (Z-)
    emit_quad(
        Vector3(local_max.x, local_min.y, local_min.z),
        Vector3(local_min.x, local_min.y, local_min.z),
        Vector3(local_min.x, local_max.y, local_min.z),
        Vector3(local_max.x, local_max.y, local_min.z),
        Vector3(0, 0, -1)
    );

    // 7. Доставка результата в базовый класс (построение ArrayMesh и вызов WorldMesh::complete_mesh)
    submit(shape_id, chunk_id, b);
}