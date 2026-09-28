#include "Chunk.hpp"
#include <godot_cpp/variant/aabb.hpp>

using namespace godot;

Chunk::Chunk(Vector3 c, float s, int d, Chunk* p)
    : center(c), size(s), depth(d), parent(p) {}

Chunk::~Chunk() {
    // Очистка мешей перед уничтожением чанка
    clear_meshes();
}

AABB Chunk::get_aabb() const {
    float half = size / 2.0f;
    return AABB(center - Vector3(half, half, half), Vector3(size, size, size));
}

void Chunk::add_mesh_instance(MeshInstance3D* mesh) {
    if (mesh) {
        mesh_instances.push_back(mesh);
        // ВАЖНО: Мы НЕ вызываем add_child здесь. 
        // Добавлением в сцену будет заниматься SpatialStreaming или WorldMesh.
    }
}

void Chunk::clear_meshes() {
    for (auto* mesh : mesh_instances) {
        if (mesh) {
            mesh->queue_free();
        }
    }
    mesh_instances.clear();
}