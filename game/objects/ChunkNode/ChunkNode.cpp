#include "ChunkNode.hpp"
#include <PlanetNode/PlanetNode.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>

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
    chunk_mesh = memnew(ChunkMesh);
    add_child(chunk_mesh);

    // Генерация меша
    build_mesh();
}


void ChunkNode::build_mesh() {
    if(!planet_data.is_valid()) return;

    chunk_mesh->build(planet_data, this);
    Ref<ArrayMesh> mesh = chunk_mesh->get_mesh();
    chunk_collider->set_mesh(mesh);

    apply_material();
}

void ChunkNode::trans_metter(const Vector3& world_pos, float delta, float radius) {
    Vector3 local = world_pos - get_global_position();

    int r = (int)Math::ceil(radius);
    Vector3i center = Vector3i(
        (int)Math::floor(local.x),
        (int)Math::floor(local.y),
        (int)Math::floor(local.z)
    );

    bool changed = false;

    for (int dx = -r; dx <= r; dx++) {
    for (int dy = -r; dy <= r; dy++) {
    for (int dz = -r; dz <= r; dz++) {

        float dist = Vector3(dx, dy, dz).length();
        if (dist > radius) continue;

        float t = dist / radius;
        float falloff = 1.0f - (t * t * (3.0f - 2.0f * t));

        Vector3i planet_voxel = origin + center + Vector3i(dx, dy, dz);

        float d = planet_data->get_block(planet_voxel, 1);
        planet_data->set_block(planet_voxel, 1, d + delta * falloff);
        changed = true;
    }}}

    if (!changed) return;

    // Перестраиваем себя + все 26 соседей (3x3x3 куб)
    if (planet_node == nullptr) {
        build_mesh();
        return;
    }

    int size = voxel_count * lod;

    for (int nx = -1; nx <= 1; nx++) {
    for (int ny = -1; ny <= 1; ny++) {
    for (int nz = -1; nz <= 1; nz++) {

        Vector3i neighbor_origin = origin + Vector3i(nx, ny, nz) * size;
        ChunkNode* chunk = planet_node->get_chunk_by_origin(neighbor_origin);
        if (chunk != nullptr) {
            chunk->build_mesh();
        }
    }}}
}

void ChunkNode::apply_material() {
    const String p_texture_path = "res://assets/snow.webp";

    Ref<StandardMaterial3D> mat;
    mat.instantiate();

    // Загружаем текстуру
    Ref<Texture2D> albedo = ResourceLoader::get_singleton()->load(p_texture_path, "Texture2D");
    if (albedo.is_valid()) {
        mat->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, albedo);
    } else {
        WARN_PRINT("ChunkNode::apply_material — не удалось загрузить текстуру: " + p_texture_path);
        mat->set_albedo(Color(0.6f, 0.55f, 0.5f)); // fallback-цвет
    }

    mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_PER_PIXEL);
    mat->set_diffuse_mode(StandardMaterial3D::DIFFUSE_BURLEY);
    mat->set_specular_mode(StandardMaterial3D::SPECULAR_SCHLICK_GGX);

    int surface_count = chunk_mesh->get_surface_override_material_count();
    for (int i = 0; i < surface_count; i++) {
        chunk_mesh->set_surface_override_material(i, mat);
    }
}

int ChunkNode::get_neighbor_lod(const Vector3i& direction) const {
    if (!planet_node) return 0;
    Vector3i neighbor_origin = origin + direction * voxel_count;
    ChunkNode* neighbor = planet_node->get_chunk_by_origin(neighbor_origin);
    return neighbor ? neighbor->get_lod() : 0;
}