#include "NeochunkNode.hpp"

#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>

using namespace godot;

void NeochunkNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &NeochunkNode::set_mesh);
}

NeochunkNode::NeochunkNode() {}

NeochunkNode::~NeochunkNode() {}

void NeochunkNode::add_debug_box() {
    MeshInstance3D* debug_mesh_instance = memnew(MeshInstance3D);

    BoxMesh* box_mesh = memnew(BoxMesh);
    box_mesh->set_size(Vector3(1.0, 1.0, 1.0) * chunk_size);
    Ref<Mesh> mesh = Ref<Mesh>(box_mesh);
    debug_mesh_instance->set_mesh(mesh);

    // Create a custom shader with wireframe mode
    Ref<Shader> shader = Ref<Shader>(memnew(Shader));
    shader->set_code(R"(
        shader_type spatial;
        render_mode wireframe, unshaded;
        
        uniform vec4 line_color : source_color;
        
        void fragment() {
            ALBEDO = line_color.rgb;
        }
    )");
    
    // Create a ShaderMaterial and assign the shader
    Ref<ShaderMaterial> debug_material = Ref<ShaderMaterial>(memnew(ShaderMaterial));
    debug_material->set_shader(shader);
    debug_material->set_shader_parameter("line_color", Color(0.2f, 0.5f, 1.0f));
    
    debug_mesh_instance->set_material_override(debug_material);
    add_child(debug_mesh_instance);
}

void NeochunkNode::set_debug_material() {
    Ref<StandardMaterial3D> material;
    material.instantiate();
    material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
    mesh_instance->set_material_override(material);
}

void NeochunkNode::set_material() {
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

    float mat_id  = round(COLOR.r * 255.0);           // дробный ID с интерполяции
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

    int surface_count = mesh_instance->get_surface_override_material_count();
    for (int i = 0; i < surface_count; i++) {
        mesh_instance->set_surface_override_material(i, mat);
    }
}

void NeochunkNode::generate_mesh() {
    if (!block_source.is_valid()) {
        print_error("NeochunkNode::generate_mesh(): block_source is not valid!");
        return;
    }

    // Только лёгкая подготовка контекста — в главном потоке
    Vector3 chunk_pos = get_position();
    float half_chunk_size = chunk_size / 2;

    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level    = lod_level;
    inp->voxel_count  = voxel_count;
    inp->chunk_size   = chunk_size;
    inp->chunk_coord  = Vector3i(
        (int)(chunk_pos.x - half_chunk_size),
        (int)(chunk_pos.y - half_chunk_size),
        (int)(chunk_pos.z - half_chunk_size)
    );

    _task.input   = inp;
    _task.task_id = WorkerThreadPool::get_singleton()->add_task(
        callable_mp_static(&NeochunkNode::_build_mesh_task).bind(get_instance_id()),
        false,
        "NeochunkNode::generate_mesh"
    );
}

void NeochunkNode::_build_mesh_task(uint64_t chunk_id) {
    NeochunkNode* chunk = Object::cast_to<NeochunkNode>(ObjectDB::get_instance(chunk_id));
    if(!chunk) return;
    auto task = chunk->_task;
    auto inp = task.input;

    if (!inp) {
        chunk->call_deferred("set_mesh", Ref<Mesh>());
        return;
    }

    // Сбор блоков — теперь в воркере
    const int stride = inp->voxel_count + 4;
    const int step   = 1 << inp->lod_level;
    chunk->block_source->fill_chunk(inp->blocks, inp->chunk_coord, stride, step);

    const MeshData data = build_neochunk_mesh(*inp);

    if (data.vertices.is_empty()) {
        chunk = Object::cast_to<NeochunkNode>(ObjectDB::get_instance(chunk_id));
        if (chunk) {
            chunk->call_deferred("set_mesh", Ref<Mesh>());
        }
        return;
    }

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = data.vertices;
    arrays[Mesh::ARRAY_NORMAL] = data.normals;
    arrays[Mesh::ARRAY_COLOR]  = data.colors;

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    chunk = Object::cast_to<NeochunkNode>(ObjectDB::get_instance(chunk_id));
    if (chunk) {
        chunk->call_deferred("set_mesh", mesh);
    }
}

void NeochunkNode::set_mesh(Ref<Mesh> mesh) {
    if(!mesh.is_valid()) return;
    // print_line("chunk_size = ", chunk_size);
    // print_line("voxel_count = ", voxel_count);
    // print_line("lod = ", lod_level);
    // print_line("mesh expected size = ", voxel_count * (1 << lod_level));

    Ref<ConcavePolygonShape3D> shape;
    shape.instantiate();
    shape->set_faces(mesh->get_faces());
    
    mesh_instance->set_mesh(mesh);
    collision_shape->set_shape(shape);

    // add_debug_box();
    // set_debug_material();
    set_material();
}

void NeochunkNode::_ready() {
    // print_line(" - realy spawn");

    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    collision_shape = memnew(CollisionShape3D);
    add_child(collision_shape);

    generate_mesh();
}