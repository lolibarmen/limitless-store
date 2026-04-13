#include "NeochunkNode.hpp"

#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
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
    box_mesh->set_size(Vector3(1.0, 1.0, 1.0) * chunk_size * 0.95);
    Ref<Mesh> mesh = Ref<Mesh>(box_mesh);

    // Создаём материал для отладки
    StandardMaterial3D* debug_material = memnew(StandardMaterial3D);
    
    // Голубой цвет Valve Debug (примерные значения RGB)
    Color debug_color(0.2f, 0.5f, 1.0f);
    debug_material->set_albedo(debug_color);
    
    // Полупрозрачность: альфа-канал 0.3–0.5 (30–50 % непрозрачности)
    debug_material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
    debug_color.a = 0.4f;  // Устанавливаем альфа-канал
    debug_material->set_albedo(debug_color);
    
    // Отключаем взаимодействие со светом
    debug_material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    
    // Применяем материал к мешу
    debug_mesh_instance->set_material_override(debug_material);
    debug_mesh_instance->set_mesh(mesh);
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

    Ref<ConcavePolygonShape3D> shape;
    shape.instantiate();
    shape->set_faces(mesh->get_faces());
    
    mesh_instance->set_mesh(mesh);
    collision_shape->set_shape(shape);
}

void NeochunkNode::_ready() {
    // print_line(" - realy spawn");

    mesh_instance = memnew(MeshInstance3D);
    add_child(mesh_instance);

    collision_shape = memnew(CollisionShape3D);
    add_child(collision_shape);

    set_debug_material();
    // add_debug_box();
    generate_mesh();
}