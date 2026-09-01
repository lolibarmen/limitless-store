#include "ChunkMaterialManager.hpp"
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/shader_material.hpp>

using namespace godot;

static const char* SHADER_CODE = R"(
shader_type spatial;
render_mode diffuse_toon, specular_toon, cull_back;

uniform int grass_material_id = 3;
uniform sampler2D grass_texture : source_color, filter_nearest_mipmap;
uniform sampler2D other_texture : source_color, filter_nearest_mipmap;
uniform float triplanar_scale = 1.0;

varying vec3 world_position;
varying vec3 world_normal;

void vertex() {
    world_position = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    world_normal = normalize((MODEL_MATRIX * vec4(NORMAL, 0.0)).xyz);
}

vec3 triplanar_sample(sampler2D tex, vec3 position, vec3 normal) {
    vec3 weights = abs(normal);
    weights = pow(weights, vec3(4.0));
    weights /= max(dot(weights, vec3(1.0)), 0.0001);

    vec2 uv_x = position.zy;
    vec2 uv_y = position.xz;
    vec2 uv_z = position.xy;

    if (normal.x < 0.0) {
        uv_x.x = -uv_x.x;
    }

    if (normal.y < 0.0) {
        uv_y.x = -uv_y.x;
    }

    if (normal.z < 0.0) {
        uv_z.x = -uv_z.x;
    }

    return
        texture(tex, uv_x * triplanar_scale).rgb * weights.x +
        texture(tex, uv_y * triplanar_scale).rgb * weights.y +
        texture(tex, uv_z * triplanar_scale).rgb * weights.z;
}

void fragment() {
    float id = round(COLOR.r * 255.0);

    vec3 normal = normalize(world_normal);

    vec3 grass_albedo = triplanar_sample(
        grass_texture,
        world_position,
        normal
    );

    vec3 other_albedo = triplanar_sample(
        other_texture,
        world_position,
        normal
    );

    ALBEDO = abs(id - float(grass_material_id)) < 0.5
        ? grass_albedo
        : other_albedo;

    ROUGHNESS = 0.9;
    METALLIC = 0.0;
}
)";

void ChunkMaterialManager::initialize() {
    if (_material.is_valid()) return;

    Ref<Shader> shader;
    shader.instantiate();
    shader->set_code(SHADER_CODE);

    Ref<ShaderMaterial> mat;
    mat.instantiate();
    mat->set_shader(shader);

    ResourceLoader* loader = ResourceLoader::get_singleton();

    Ref<Texture2D> grass_tex = loader->load("res://assets/Chunk/grass.png");
    Ref<Texture2D> other_tex = loader->load("res://assets/Chunk/null.webp");

    mat->set_shader_parameter("grass_material_id", 3);
    mat->set_shader_parameter("grass_texture", grass_tex);
    mat->set_shader_parameter("other_texture", other_tex);
    mat->set_shader_parameter("triplanar_scale", 1.0f);

    _material = mat;
}