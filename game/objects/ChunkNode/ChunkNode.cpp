#include "ChunkNode.hpp"
#include <ChunkManager/ChunkManager.hpp>

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
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ChunkNode::set_mesh);
}

void ChunkNode::init(
    ChunkManager* p_manager,
    Ref<BlockSource> p_source,
    const Vector3i &p_origin,
    int p_voxel_count,
    int p_lod)
{
    chunk_manager = p_manager,
    block_source = p_source;
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
    if(!block_source.is_valid()) return;
    chunk_mesh->build(block_source, this);
}

void ChunkNode::set_mesh(Ref<ArrayMesh> p_mesh) {
    chunk_mesh->set_mesh(p_mesh);
    chunk_collider->set_mesh(p_mesh);
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

        BlockData block_data = block_source->get_block(planet_voxel);
        block_data.density += delta * falloff;

        block_source->set_block(planet_voxel, block_data);
        changed = true;
    }}}

    if (!changed) return;

    // Перестраиваем себя + все 26 соседей (3x3x3 куб)
    if (chunk_manager == nullptr) {
        build_mesh();
        return;
    }

    int size = voxel_count * lod;

    for (int nx = -1; nx <= 1; nx++) {
    for (int ny = -1; ny <= 1; ny++) {
    for (int nz = -1; nz <= 1; nz++) {

        Vector3i neighbor_origin = origin + Vector3i(nx, ny, nz) * size;
        ChunkNode* chunk = chunk_manager->get_chunk_by_origin(neighbor_origin);
        if (chunk != nullptr) {
            chunk->build_mesh();
        }
    }}}
}

void ChunkNode::apply_material() {
    const String p_texture_path = "res://assets/grass.webp";

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
    if (!chunk_manager) return 0;
    Vector3i neighbor_origin = origin + direction * voxel_count;
    ChunkNode* neighbor = chunk_manager->get_chunk_by_origin(neighbor_origin);
    return neighbor ? neighbor->get_lod() : 0;
}