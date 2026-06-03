#include "ChunkMaterialManager.hpp"
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/shader_material.hpp>

using namespace godot;

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
    if (id < 1.5) return triplanar_sample(texture_stone,  world_pos, normal);
    if (id < 2.5) return triplanar_sample(texture_dirt,   world_pos, normal);
    if (id < 3.5) return triplanar_sample(texture_grass,  world_pos, normal);
    if (id < 4.5) return triplanar_sample(texture_sand,   world_pos, normal);
    if (id < 5.5) return triplanar_sample(texture_snow,   world_pos, normal);
                  return triplanar_sample(texture_gravel, world_pos, normal);
}

void fragment() {
    vec3 world_pos = (INV_VIEW_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vec3 world_normal = normalize((INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz);

    float mat_id  = round(COLOR.r * 255.0);           // дробный ID с интерполяции
    float id_low  = floor(mat_id);    // нижний материал
    // float id_high = ceil(mat_id);     // верхний материал
    // float blend   = fract(mat_id);    // сколько верхнего

    vec4 col_low  = sample_by_id(id_low,  world_pos, world_normal);
    // vec4 col_high = sample_by_id(id_high, world_pos, world_normal);

    vec4 albedo = col_low; // mix(col_low, col_high, blend);

    ALBEDO   = albedo.rgb;
    ROUGHNESS = 0.85;
    METALLIC  = 0.0;
}
)";

void ChunkMaterialManager::initialize() {
    if (_material.is_valid()) return; // уже инициализирован

    Ref<Shader> shader;
    shader.instantiate();
    shader->set_code(SHADER_CODE);

    Ref<ShaderMaterial> mat;
    mat.instantiate();
    mat->set_shader(shader);

    auto load_tex = [](const String& path) -> Ref<Texture2D> {
        return ResourceLoader::get_singleton()->load(path, "Texture2D");
    };

    mat->set_shader_parameter("texture_grass",  load_tex("res://assets/Chunk/grass.webp"));
    mat->set_shader_parameter("texture_dirt",   load_tex("res://assets/Chunk/dirt.webp"));
    mat->set_shader_parameter("texture_stone",  load_tex("res://assets/Chunk/stone.webp"));
    mat->set_shader_parameter("texture_sand",   load_tex("res://assets/Chunk/sand.webp"));
    mat->set_shader_parameter("texture_snow",   load_tex("res://assets/Chunk/snow.webp"));
    mat->set_shader_parameter("texture_gravel", load_tex("res://assets/Chunk/gravel.webp"));

    mat->set_shader_parameter("texture_null", load_tex("res://assets/Chunk/null.webp"));

    mat->set_shader_parameter("texture_scale",    0.5f);
    mat->set_shader_parameter("blend_sharpness",  16.0f);

    _material = mat;
}