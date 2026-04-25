#include "NeochunkNode.hpp"
#include "ChunkMeshQueue.hpp"
#include "ChunkMaterialManager.hpp"
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>

using namespace godot;

void NeochunkNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &NeochunkNode::set_mesh);
    ClassDB::bind_method(D_METHOD("get_block_source"), &NeochunkNode::get_block_source);
    ClassDB::bind_method(D_METHOD("generate_mesh"), &NeochunkNode::generate_mesh);
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
    Object* obj = ObjectDB::get_instance(chunk_id);
    if(!obj) return;
    NeochunkNode* chunk = Object::cast_to<NeochunkNode>(obj);
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

    ChunkMeshQueue::get_singleton().push(chunk_id, mesh);
}

void NeochunkNode::set_mesh(Ref<Mesh> mesh) {
    if(!mesh.is_valid()) return;

    Ref<ConcavePolygonShape3D> shape;
    shape.instantiate();
    shape->set_faces(mesh->get_faces());
    
    mesh_instance->set_mesh(mesh);
    collision_shape->set_shape(shape);

    auto& mgr = ChunkMaterialManager::get_singleton();
    Ref<ShaderMaterial> mat = mgr.get_material();

    int surface_count = mesh_instance->get_surface_override_material_count();
    for (int i = 0; i < surface_count; i++) {
        mesh_instance->set_surface_override_material(i, mat);
    }
}

void NeochunkNode::_ready() {
    add_to_group("mineable");

    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    collision_shape = memnew(CollisionShape3D);
    add_child(collision_shape);

    generate_mesh();
}