#include "ChunkNode.hpp"
#include <ChunkManager/ChunkManager.hpp>
#include <ChunkMeshGenerator/ChunkMeshGenerator.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/shader_material.hpp>

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
    Ref<BlockLODSource> p_source,
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
    chunk_mesh = memnew(MeshInstance3D);
    add_child(chunk_mesh);

    build_mesh();
}


void ChunkNode::build_mesh() {
    if(!block_source.is_valid()) {
        print_error("void ChunkNode::build_mesh() block_source not valid");
        return;
    }
    
    Ref<ChunkMeshGenerator> chunk_mesh_generator;
    chunk_mesh_generator.instantiate();
    chunk_mesh_generator->build(block_source, this);
    chunk_mesh_generator.unref();
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
    // Шейдер читает COLOR.r как ID материала (1=grass, 2=dirt, 3=stone, ...)
    // и делает triplanar-блендинг между двумя ближайшими текстурами
    static const char* SHADER_CODE = R"(
shader_type spatial;

uniform sampler2D texture_grass   : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_dirt    : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_stone   : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_sand    : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_snow    : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_gravel  : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_null  : source_color, filter_linear_mipmap, repeat_enable;

uniform float texture_scale : hint_range(0.01, 10.0) = 0.5;
uniform float blend_sharpness : hint_range(1.0, 16.0) = 4.0;

vec4 triplanar_sample(sampler2D tex, vec3 world_pos, vec3 normal) {
    vec3 blending = pow(abs(normal), vec3(blend_sharpness));
    blending /= (blending.x + blending.y + blending.z);

    vec4 x = texture(tex, world_pos.yz * texture_scale);
    vec4 y = texture(tex, world_pos.xz * texture_scale);
    vec4 z = texture(tex, world_pos.xy * texture_scale);
    return x * blending.x + y * blending.y + z * blending.z;
}

vec4 sample_by_id(float id, vec3 world_pos, vec3 normal) {
    if (id < 0.5) return triplanar_sample(texture_null,   world_pos, normal);
    if (id < 1.5) return triplanar_sample(texture_grass,  world_pos, normal);
    if (id < 2.5) return triplanar_sample(texture_dirt,   world_pos, normal);
    if (id < 3.5) return triplanar_sample(texture_stone,  world_pos, normal);
    if (id < 4.5) return triplanar_sample(texture_sand,   world_pos, normal);
    if (id < 5.5) return triplanar_sample(texture_snow,   world_pos, normal);
                  return triplanar_sample(texture_gravel, world_pos, normal);
}

void fragment() {
    vec3 world_pos = (INV_VIEW_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vec3 world_normal = normalize((INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz);

    float mat_id = COLOR.r;           // дробный ID с интерполяции
    float id_low  = floor(mat_id);    // нижний материал
    float id_high = ceil(mat_id);     // верхний материал
    float blend   = fract(mat_id);    // сколько верхнего

    vec4 col_low  = sample_by_id(id_low,  world_pos, world_normal);
    vec4 col_high = sample_by_id(id_high, world_pos, world_normal);

    vec4 albedo = col_low; // mix(col_low, col_high, blend);

    ALBEDO   = albedo.rgb;
    ROUGHNESS = 0.85;
    METALLIC  = 0.0;
}
)";

    Ref<Shader> shader;
    shader.instantiate();
    shader->set_code(SHADER_CODE);

    Ref<ShaderMaterial> mat;
    mat.instantiate();
    mat->set_shader(shader);

    // Загружаем текстуры
    auto load_tex = [](const String& path) -> Ref<Texture2D> {
        return ResourceLoader::get_singleton()->load(path, "Texture2D");
    };

    mat->set_shader_parameter("texture_grass",  load_tex("res://assets/grass.webp"));
    mat->set_shader_parameter("texture_dirt",   load_tex("res://assets/dirt.webp"));
    mat->set_shader_parameter("texture_stone",  load_tex("res://assets/stone.webp"));
    mat->set_shader_parameter("texture_sand",   load_tex("res://assets/sand.webp"));
    mat->set_shader_parameter("texture_snow",   load_tex("res://assets/snow.webp"));
    mat->set_shader_parameter("texture_gravel", load_tex("res://assets/gravel.webp"));

    mat->set_shader_parameter("texture_null", load_tex("res://assets/null.webp"));

    mat->set_shader_parameter("texture_scale",    0.5f);
    mat->set_shader_parameter("blend_sharpness",  4.0f);

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