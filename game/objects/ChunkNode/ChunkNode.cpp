#include "ChunkNode.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {
    // Направления осей (положительные)
    const Vector3i AXIS[] = {
        Vector3i(1, 0, 0),
        Vector3i(0, 1, 0),
        Vector3i(0, 0, 1)
    };

    // Углы квадрата для каждой оси
    const Vector3i QUAD_POINTS[3][4] = {
        // Ось X
        {
            Vector3i(0, 0, -1),
            Vector3i(0, -1, -1),
            Vector3i(0, -1, 0),
            Vector3i(0, 0, 0)
        },
        // Ось Y
        {
            Vector3i(0, 0, -1),
            Vector3i(0, 0, 0),
            Vector3i(-1, 0, 0),
            Vector3i(-1, 0, -1)
        },
        // Ось Z
        {
            Vector3i(0, 0, 0),
            Vector3i(0, -1, 0),
            Vector3i(-1, -1, 0),
            Vector3i(-1, 0, 0)
        }
    };

    // 12 рёбер куба (пары точек)
    const Vector3i EDGE_OFFSETS[12][2] = {
        // Рёбра на min Z
        { Vector3i(0, 0, 0), Vector3i(1, 0, 0) },
        { Vector3i(1, 0, 0), Vector3i(1, 1, 0) },
        { Vector3i(1, 1, 0), Vector3i(0, 1, 0) },
        { Vector3i(0, 1, 0), Vector3i(0, 0, 0) },
        // Рёбра на max Z
        { Vector3i(0, 0, 1), Vector3i(1, 0, 1) },
        { Vector3i(1, 0, 1), Vector3i(1, 1, 1) },
        { Vector3i(1, 1, 1), Vector3i(0, 1, 1) },
        { Vector3i(0, 1, 1), Vector3i(0, 0, 1) },
        // Рёбра, соединяющие min Z и max Z
        { Vector3i(0, 0, 0), Vector3i(0, 0, 1) },
        { Vector3i(1, 0, 0), Vector3i(1, 0, 1) },
        { Vector3i(1, 1, 0), Vector3i(1, 1, 1) },
        { Vector3i(0, 1, 0), Vector3i(0, 1, 1) }
    };
}

ChunkNode::ChunkNode() :
        mesh_instance(nullptr),
        collision_shape(nullptr) {
}

ChunkNode::~ChunkNode() {
}

void ChunkNode::_bind_methods() {
}

void ChunkNode::_ready() {    
    // Создаём StaticBody3D как дочерний узел
    static_body = memnew(StaticBody3D);
    add_child(static_body);

    // Создаём MeshInstance3D как дочерний узел
    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    // Создаём CollisionShape3D как дочерний узел StaticBody3D
    collision_shape = memnew(CollisionShape3D);
    static_body->add_child(collision_shape);

    // Генерация меша
    surface_tool.instantiate();
    surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

    create_surface_mesh(2); // размер области = 6

    surface_tool->generate_normals();
    Ref<ArrayMesh> mesh = surface_tool->commit();
    mesh_instance->set_mesh(mesh);
    
    // Настройка коллизии (выпуклая оболочка на основе меша)
    if (mesh.is_valid()) {
        Ref<ConcavePolygonShape3D> shape;
        shape.instantiate();
        shape->set_faces(mesh->get_faces());
        collision_shape->set_shape(shape);
    }

    surface_tool.unref();
}

void ChunkNode::set_planet_data(Ref<PlanetData> p_data) {
    planet_data = p_data;
}

float ChunkNode::get_density(const Vector3i &p_index) const {
    float dens = planet_data->get_block(p_index);
    return planet_data->get_block(p_index);

    // Vector3 pos(p_index.x, p_index.y, p_index.z);
    // return pos.distance_to(Vector3(0.0, 0.0, 0.0)) - 2.0;
}

void ChunkNode::create_surface_mesh(int p_size) {
    // Перебираем все целочисленные координаты в кубе [-size, size-1]
    for (int x = -p_size; x < p_size; ++x) {
        for (int y = -p_size; y < p_size; ++y) {
            for (int z = -p_size; z < p_size; ++z) {
                create_surface_mesh_quad(Vector3i(x, y, z));
            }
        }
    }
}

void ChunkNode::create_surface_mesh_quad(const Vector3i &p_index) {
    for (int axis_index = 0; axis_index < 3; ++axis_index) {
        const Vector3i &axis = AXIS[axis_index];
        float val1 = get_density(p_index);
        float val2 = get_density(p_index + axis);

        if (val1 > 0 && val2 <= 0) {
            add_quad(p_index, axis_index);
        } else if (val1 <= 0 && val2 > 0) {
            add_reversed_quad(p_index, axis_index);
        }
    }
}

void ChunkNode::add_quad(const Vector3i &p_index, int p_axis_index) {
    Vector3i points[4];
    for (int i = 0; i < 4; ++i) {
        points[i] = p_index + QUAD_POINTS[p_axis_index][i];
    }

    // Первый треугольник
    add_vertex(points[0]);
    add_vertex(points[1]);
    add_vertex(points[2]);

    // Второй треугольник
    add_vertex(points[0]);
    add_vertex(points[2]);
    add_vertex(points[3]);
}

void ChunkNode::add_reversed_quad(const Vector3i &p_index, int p_axis_index) {
    Vector3i points[4];
    for (int i = 0; i < 4; ++i) {
        points[i] = p_index + QUAD_POINTS[p_axis_index][i];
    }

    // Реверсивный порядок вершин
    add_vertex(points[0]);
    add_vertex(points[2]);
    add_vertex(points[1]);

    add_vertex(points[0]);
    add_vertex(points[3]);
    add_vertex(points[2]);
}

Vector3 ChunkNode::get_surface_position(const Vector3i &p_index) const {
    Vector3 total(0, 0, 0);
    int edge_count = 0;

    for (int i = 0; i < 12; ++i) {
        Vector3i a = p_index + EDGE_OFFSETS[i][0];
        Vector3i b = p_index + EDGE_OFFSETS[i][1];
        float va = get_density(a);
        float vb = get_density(b);

        // Если знаки разные — пересечение
        if (va * vb < 0) {
            edge_count++;
            float t = Math::abs(va) / (Math::abs(va) + Math::abs(vb));
            Vector3 pos_a(a.x, a.y, a.z);
            Vector3 pos_b(b.x, b.y, b.z);
            total += pos_a.lerp(pos_b, t);
        }
    }

    if (edge_count == 0) {
        // Если нет пересечений, возвращаем центр вокселя
        return Vector3(p_index.x, p_index.y, p_index.z) + Vector3(0.5f, 0.5f, 0.5f);
    }

    return total / edge_count;
}

Vector3 ChunkNode::get_surface_gradient(const Vector3i &p_index, float p_sample_value) const {
    float dx = p_sample_value - get_density(p_index + AXIS[0]);
    float dy = p_sample_value - get_density(p_index + AXIS[1]);
    float dz = p_sample_value - get_density(p_index + AXIS[2]);
    return Vector3(dx, dy, dz).normalized();
}

void ChunkNode::add_vertex(const Vector3i &p_index) {
    float val = get_density(p_index);
    Vector3 pos = get_surface_position(p_index);
    // Vector3 normal = get_surface_gradient(p_index, val);

    // surface_tool->set_normal(normal);
    surface_tool->add_vertex(pos);
}