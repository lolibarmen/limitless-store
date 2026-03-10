#include "ChunkNode.hpp"
#include <PlanetNode/PlanetNode.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/standard_material3d.hpp>

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
    int p_lod)
{
    planet_node = p_planet,
    planet_data = p_data;
    origin = p_origin;
    voxel_count = p_voxel_count;
    lod = p_lod;
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

    apply_debug_material();
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

void ChunkNode::apply_debug_material() {
    if (!mesh_instance || !mesh_instance->get_mesh().is_valid()) return;
    if (mesh_instance->get_mesh()->get_surface_count() == 0) return;

    Ref<StandardMaterial3D> mat;
    mat.instantiate();

    switch (lod) {
        case 1: mat->set_albedo(Color(0.0f, 1.0f, 0.0f)); break; // Зелёный   — LOD 1 (близко)
        case 2: mat->set_albedo(Color(1.0f, 1.0f, 0.0f)); break; // Жёлтый    — LOD 2
        case 4: mat->set_albedo(Color(1.0f, 0.5f, 0.0f)); break; // Оранжевый — LOD 4
        case 8: mat->set_albedo(Color(1.0f, 0.0f, 0.0f)); break; // Красный   — LOD 8 (далеко)
        default: mat->set_albedo(Color(1.0f, 1.0f, 1.0f)); break; // Белый    — неизвестный
    }

    mesh_instance->set_surface_override_material(0, mat);
}