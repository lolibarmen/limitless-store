#include "ChunkNode.hpp"
#include <PlanetNode/PlanetNode.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ChunkNode::ChunkNode() {
}

ChunkNode::~ChunkNode() {
}

void ChunkNode::_bind_methods() {
}

void ChunkNode::configure(
    PlanetNode* p_planet,
    Ref<PlanetData> p_data,
    const Vector3i &p_origin,
    int p_voxel_count,
    int p_sample_step)
{
    planet_node = p_planet,
    planet_data = p_data;
    origin = p_origin;
    voxel_count = p_voxel_count;
    sample_step = p_sample_step;
}

void ChunkNode::_ready() {
    // Создаём StaticBody3D как дочерний узел
    chunk_collider = memnew(ChunkCollider);
    chunk_collider->set_chunk(this);
    add_child(chunk_collider);

    // Создаём MeshInstance3D как дочерний узел
    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    // Генерация меша
    build_mesh();
}

void ChunkNode::build_mesh() {
    if(!planet_data.is_valid()) return;

    chunk_mesh.instantiate();

    Ref<ArrayMesh> mesh = chunk_mesh->build(planet_data, this);
    
    mesh_instance->set_mesh(mesh);
    chunk_collider->set_mesh(mesh);
    
    chunk_mesh.unref();
}

void ChunkNode::on_ray_hit(const Dictionary &result) {
    
    Vector3 world_pos = result["position"];

    Vector3 local = world_pos - get_global_position();

    Vector3i voxel = Vector3i(
        Math::floor(local.x),
        Math::floor(local.y),
        Math::floor(local.z)
    );

    Vector3i planet_voxel = origin + voxel;

    planet_node->on_block_hit(planet_voxel);

    build_mesh();
}