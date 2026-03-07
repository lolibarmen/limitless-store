#include "ChunkMesh.hpp"
#include <ChunkNode/ChunkNode.hpp>

using namespace godot;

#include "cube_vertex.h"

ChunkMesh::ChunkMesh() {}

ChunkMesh::~ChunkMesh() {}

void ChunkMesh::_bind_methods() {

}

void ChunkMesh::create_surface_mesh(int p_size) {
    // Перебираем все целочисленные координаты в кубе [-size, size-1]
    for (int x = -p_size; x < p_size; ++x) {
        for (int y = -p_size; y < p_size; ++y) {
            for (int z = -p_size; z < p_size; ++z) {
                create_surface_mesh_quad(Vector3i(x, y, z));
            }
        }
    }
}

void ChunkMesh::create_surface_mesh_quad(const Vector3i &p_index) {
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

void ChunkMesh::add_quad(const Vector3i &p_index, int p_axis_index) {
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

void ChunkMesh::add_reversed_quad(const Vector3i &p_index, int p_axis_index) {
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

Vector3 ChunkMesh::get_surface_position(const Vector3i &p_index) const {
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

Vector3 ChunkMesh::get_surface_gradient(const Vector3i &p_index, float p_sample_value) const {
    float dx = p_sample_value - get_density(p_index + AXIS[0]);
    float dy = p_sample_value - get_density(p_index + AXIS[1]);
    float dz = p_sample_value - get_density(p_index + AXIS[2]);
    return Vector3(dx, dy, dz).normalized();
}

void ChunkMesh::add_vertex(const Vector3i &p_index) {
    Vector3 pos_sum(0, 0, 0);
    Vector3 normal_sum(0, 0, 0);
    int count = 0;

    // Проходим по всем 12 рёбрам вокселя
    for (int i = 0; i < 12; ++i) {
        Vector3i a = p_index + EDGE_OFFSETS[i][0];
        Vector3i b = p_index + EDGE_OFFSETS[i][1];
        float va = get_density(a);
        float vb = get_density(b);

        // Если знаки разные — пересечение
        if (va * vb < 0) {
            count++;
            float t = Math::abs(va) / (Math::abs(va) + Math::abs(vb));
            Vector3 pos_a(a.x, a.y, a.z);
            Vector3 pos_b(b.x, b.y, b.z);
            Vector3 pos_edge = pos_a.lerp(pos_b, t);
            pos_sum += pos_edge;

            // Нормали в вершинах ребра (используем градиент в узлах)
            Vector3 normal_a = get_surface_gradient(a, va);
            Vector3 normal_b = get_surface_gradient(b, vb);
            // Интерполируем нормаль вдоль ребра и нормализуем
            Vector3 normal_edge = normal_a.lerp(normal_b, t).normalized();
            normal_sum += normal_edge;
        }
    }

    // Если пересечений нет (редкий случай), берём центр вокселя
    if (count == 0) {
        pos_sum = Vector3(p_index.x, p_index.y, p_index.z) + Vector3(0.5f, 0.5f, 0.5f);
        normal_sum = get_surface_gradient(p_index, get_density(p_index));
        count = 1;
    }

    Vector3 final_pos = pos_sum / count;
    Vector3 final_normal = normal_sum.normalized();

    st->set_normal(final_normal);
    st->add_vertex(final_pos);
}

Ref<ArrayMesh> ChunkMesh::build(Ref<PlanetData> p_data, ChunkNode* p_chunk) {
    chunk = p_chunk;
    planet_data = p_data;

    st.instantiate();

    st->begin(Mesh::PRIMITIVE_TRIANGLES);
    create_surface_mesh(chunk->get_voxel_count());
    Ref<ArrayMesh> mesh = st->commit();

    st.unref();

    return mesh;
}

float ChunkMesh::get_density(const Vector3i &p_index) const {
    Vector3i world_index = chunk->get_origin() + p_index * chunk->get_sample_step();
    return planet_data->get_density(world_index, chunk->get_sample_step());
}