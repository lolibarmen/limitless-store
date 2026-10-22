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