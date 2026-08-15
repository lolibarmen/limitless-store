#include "ChunkMeshNode.hpp"

using namespace godot;

void ChunkMeshNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ChunkMeshNode::set_mesh);
    ClassDB::bind_method(D_METHOD("generate_mesh"), &ChunkMeshNode::generate_mesh);
}

ChunkMeshNode::ChunkMeshNode() {}
ChunkMeshNode::~ChunkMeshNode() {}

void ChunkMeshNode::_ready() {
    // Создаем визуальные и физические узлы как детей этого StaticBody3D
    _mesh_instance = memnew(MeshInstance3D);
    add_child(_mesh_instance);

    _collision_shape = memnew(CollisionShape3D);
    add_child(_collision_shape);
}

void ChunkMeshNode::generate_mesh() {
    if (!_block_source.is_valid()) {
        print_error("ChunkMeshNode::generate_mesh(): block_source is not valid!");
        return;
    }

    Vector3 chunk_pos = get_global_position();
    float half_chunk_size = _chunk_size / 2;

    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level    = _lod_level;
    inp->voxel_count  = _voxel_count;
    inp->chunk_size   = _chunk_size;

    inp->chunk_coord  = Vector3i(
        (int)(chunk_pos.x - half_chunk_size),
        (int)(chunk_pos.y - half_chunk_size),
        (int)(chunk_pos.z - half_chunk_size)
    );

    _task.input   = inp;
    _task.task_id = WorkerThreadPool::get_singleton()->add_task(
        callable_mp_static(&ChunkMeshNode::_build_mesh_task).bind(get_instance_id()),
        false,
        "ChunkMeshNode::generate_mesh"
    );
}

void ChunkMeshNode::_build_mesh_task(uint64_t node_id) {
    Object* obj = ObjectDB::get_instance(node_id);
    ChunkMeshNode* node = Object::cast_to<ChunkMeshNode>(obj);
    if (!node) return;

    auto task = node->_task;
    auto inp = task.input;

    if (!inp) {
        node->call_deferred("set_mesh", Ref<Mesh>());
        return;
    }

    const int stride = inp->voxel_count + 4;
    const int step   = 1 << inp->lod_level;

    // Сбор блоков теперь использует безопасную копию Ref из inp
    node->_block_source->fill_chunk(inp->blocks, inp->chunk_coord, stride, step);

    const MeshData data = build_neochunk_mesh(*inp);

    if (data.vertices.is_empty()) {
        node->call_deferred("set_mesh", Ref<Mesh>());
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

    // Отдаем в очередь ID этого узла (ChunkMeshNode)
    ChunkMeshQueue::get_singleton().push(node_id, mesh);
}

void ChunkMeshNode::set_mesh(Ref<Mesh> mesh) {
    if (!mesh.is_valid()) return;
    if (!_mesh_instance || !_collision_shape) return;

    Ref<ConcavePolygonShape3D> shape;
    shape.instantiate();
    shape->set_faces(mesh->get_faces());

    _mesh_instance->set_mesh(mesh);
    _collision_shape->set_shape(shape);

    auto& mgr = ChunkMaterialManager::get_singleton();
    Ref<ShaderMaterial> mat = mgr.get_material();

    int surface_count = _mesh_instance->get_surface_override_material_count();
    for (int i = 0; i < surface_count; i++) {
        _mesh_instance->set_surface_override_material(i, mat);
    }
}

void ChunkMeshNode::set_debug_material() {
    if (!_mesh_instance) return;
    Ref<StandardMaterial3D> material;
    material.instantiate();
    material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
    _mesh_instance->set_material_override(material);
}