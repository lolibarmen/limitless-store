#include "ChunkMaterialManager.hpp"
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/shader_material.hpp>

using namespace godot;

static const char* SHADER_CODE = R"(
shader_type spatial;

uniform sampler2D texture_grass   : source_color, filter_nearest, repeat_enable;
uniform sampler2D texture_dirt    : source_color, filter_nearest, repeat_enable;
uniform sampler2D texture_stone   : source_color, filter_nearest, repeat_enable;
uniform sampler2D texture_sand    : source_color, filter_nearest, repeat_enable;
uniform sampler2D texture_snow    : source_color, filter_nearest, repeat_enable;
uniform sampler2D texture_gravel  : source_color, filter_nearest, repeat_enable;
uniform sampler2D texture_null    : source_color, filter_nearest, repeat_enable;

uniform sampler2D texture_noise : filter_nearest, repeat_enable;

uniform float texture_scale;

vec4 triplanar_sample(sampler2D tex, vec3 world_pos, vec3 normal) {
    const float blend_sharpness = 8.0;
    vec3 blending = pow(abs(normal), vec3(blend_sharpness));
    blending /= (blending.x + blending.y + blending.z);
    
    float s = texture_scale;
    vec4 x = texture(tex, world_pos.yz * s);
    vec4 y = texture(tex, world_pos.xz * s);
    vec4 z = texture(tex, world_pos.xy * s);
    return x * blending.x + y * blending.y + z * blending.z;
}

vec4 sample_by_id(float id, vec3 world_pos, vec3 normal) {
    float slope = 1.0 - abs(normal.y);

    // Читаем шум из текстуры, квантованной по мировым координатам
    vec2 noise_uv = world_pos.xz * texture_scale;
    float n = texture(texture_noise, noise_uv).r;

    float steepness = slope * slope;

    if (id < 0.5) return triplanar_sample(texture_null,   world_pos, normal);
    if (id < 1.5) return triplanar_sample(texture_stone,  world_pos, normal);
    if (id < 2.5) return triplanar_sample(texture_dirt,   world_pos, normal);
    if (id < 3.5) {
        vec4 grass = triplanar_sample(texture_grass, world_pos, normal);
        vec4 dirt  = triplanar_sample(texture_dirt,  world_pos, normal);
        return (n < steepness) ? dirt : grass;
    }
    if (id < 4.5) return triplanar_sample(texture_sand,   world_pos, normal);
    if (id < 5.5) return triplanar_sample(texture_snow,   world_pos, normal);
                  return triplanar_sample(texture_gravel, world_pos, normal);
}

void fragment() {
    vec3 world_pos    = (INV_VIEW_MATRIX * vec4(VERTEX, 1.0)).xyz;
    vec3 world_normal = normalize((INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz);
    
    float id = round(COLOR.r * 255.0);
    vec4 albedo = sample_by_id(id, world_pos, world_normal);
    
    ALBEDO = albedo.rgb;
    ROUGHNESS = 0.85;
    METALLIC = 0.0;
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

    mat->set_shader_parameter("texture_null", load_tex("res://assets/Chunk/null.webp"));
    mat->set_shader_parameter("texture_grass",  load_tex("res://assets/Chunk/grass.png"));
    mat->set_shader_parameter("texture_dirt",   load_tex("res://assets/Chunk/dirt.webp"));
    mat->set_shader_parameter("texture_stone",  load_tex("res://assets/Chunk/stone.webp"));
    mat->set_shader_parameter("texture_sand",   load_tex("res://assets/Chunk/sand.webp"));
    mat->set_shader_parameter("texture_snow",   load_tex("res://assets/Chunk/snow.webp"));
    mat->set_shader_parameter("texture_gravel", load_tex("res://assets/Chunk/gravel.webp"));

    mat->set_shader_parameter("texture_noise", load_tex("res://assets/Chunk/tileable_grayscale_noise.tres"));

    mat->set_shader_parameter("texture_scale",    0.5f);

    _material = mat;
}