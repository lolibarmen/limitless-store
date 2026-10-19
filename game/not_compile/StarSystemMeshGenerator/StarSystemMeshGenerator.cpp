#include "StarSystemMeshGenerator.hpp"
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

void StarSystemMeshGenerator::_bind_methods() {}

Ref<ArrayMesh> StarSystemMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<ArrayMesh> mesh;
    mesh.instantiate();

    PackedVector3Array vertices;
    PackedVector3Array normals;
    PackedVector2Array uvs;
    PackedInt32Array indices;

    float r = _radius;
    // Вершины октаэдра (минимальная аппроксимация сферы)
    vertices.push_back(Vector3(0, r, 0));   // 0: верх
    vertices.push_back(Vector3(0, -r, 0));  // 1: низ
    vertices.push_back(Vector3(r, 0, 0));   // 2: право
    vertices.push_back(Vector3(-r, 0, 0));  // 3: лево
    vertices.push_back(Vector3(0, 0, r));   // 4: перед
    vertices.push_back(Vector3(0, 0, -r));  // 5: зад

    for (int i = 0; i < 6; i++) {
        normals.push_back(vertices[i].normalized());
        uvs.push_back(Vector2(0, 0));
    }

    // Верхняя полусфера (4 треугольника)
    indices.push_back(0); indices.push_back(4); indices.push_back(2);
    indices.push_back(0); indices.push_back(2); indices.push_back(5);
    indices.push_back(0); indices.push_back(5); indices.push_back(3);
    indices.push_back(0); indices.push_back(3); indices.push_back(4);

    // Нижняя полусфера (4 треугольника)
    indices.push_back(1); indices.push_back(2); indices.push_back(4);
    indices.push_back(1); indices.push_back(5); indices.push_back(2);
    indices.push_back(1); indices.push_back(3); indices.push_back(5);
    indices.push_back(1); indices.push_back(4); indices.push_back(3);

    Array mesh_array;
    mesh_array.resize(Mesh::ARRAY_MAX);
    mesh_array[Mesh::ARRAY_VERTEX] = vertices;
    mesh_array[Mesh::ARRAY_NORMAL] = normals;
    mesh_array[Mesh::ARRAY_TEX_UV] = uvs;
    mesh_array[Mesh::ARRAY_INDEX] = indices;

    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
    return mesh;
}