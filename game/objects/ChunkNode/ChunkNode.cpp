#include "ChunkNode.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ChunkNode::ChunkNode() :
        mesh_instance(nullptr),
        collision_shape(nullptr) {
}

ChunkNode::~ChunkNode() {
}

void ChunkNode::_bind_methods() {
}

void ChunkNode::configure(
    Ref<PlanetData> p_data,
    const Vector3i &p_origin,
    int p_voxel_count,
    int p_sample_step)
{
    planet_data = p_data;
    origin = p_origin;
    voxel_count = p_voxel_count;
    sample_step = p_sample_step;
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
    build_mesh();
}

#include <cstdio>
void ChunkNode::build_mesh() {
    if(!planet_data.is_valid()) return;

    chunk_mesh.instantiate();

    Ref<ArrayMesh> mesh = chunk_mesh->build(planet_data, this);
    
    mesh_instance->set_mesh(mesh);
    
    // Настройка коллизии (выпуклая оболочка на основе меша)
    if (mesh.is_valid()) {
        Ref<ConcavePolygonShape3D> shape;
        shape.instantiate();
        shape->set_faces(mesh->get_faces());
        collision_shape->set_shape(shape);
    }
    
    chunk_mesh.unref();
}