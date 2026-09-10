#include "SurfaceNets.hpp"
#include "ChunkMeshNode.hpp"
#include <Utils/VoxelBaker.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <ChunkMeshNode/ChunkMeshQueue.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <cstdio>

using namespace godot;

void ChunkMeshNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &ChunkMeshNode::set_mesh);
    ClassDB::bind_method(D_METHOD("generate_mesh"), &ChunkMeshNode::generate_mesh);
}

ChunkMeshNode::ChunkMeshNode() {}
ChunkMeshNode::~ChunkMeshNode() {}

void ChunkMeshNode::_ready() {
    _mesh_instance = nullptr;
    _collision_shape = nullptr;

    generate_mesh();
}

void ChunkMeshNode::generate_mesh() {
    Vector3 chunk_pos = get_global_position();
    float half_chunk_size = _chunk_size / 2.0f;

    auto inp = std::make_shared<ChunkBuildInput>();
    inp->lod_level    = _lod_level;
    inp->voxel_count  = _voxel_count;
    inp->chunk_size   = _chunk_size;
    inp->stride       = (int)_voxel_count + 4;
    inp->step         = 1 << _lod_level;
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

    inp->cache = std::make_shared<VoxelCache>(inp->stride, inp->step, inp->chunk_coord);
    
    SemanticWorld* sw = (SemanticWorld*)Engine::get_singleton()->get_singleton("SemanticWorld");
    if (sw) {
        std::vector<Ref<SemanticShape>> shapes = sw->get_shapes_snapshot();
        VoxelBaker::bake(*(inp->cache), shapes);
    }

    const MeshData data = build_neochunk_mesh(*inp);

    if (data.vertices.is_empty()) {
        node->call_deferred("set_mesh", Ref<Mesh>());
        return;
    }

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = data.vertices;
    arrays[Mesh::ARRAY_NORMAL] = data.normals;
    // arrays[Mesh::ARRAY_COLOR]  = data.colors;

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    ChunkMeshQueue::get_singleton().push(node_id, mesh);
}

void ChunkMeshNode::set_mesh(Ref<Mesh> mesh) {
    if (is_queued_for_deletion()) {
        return;
    }

    if (!mesh.is_valid()) {
        if (_mesh_instance) {
            _mesh_instance->set_mesh(Ref<Mesh>());
        }
        if (_collision_shape) {
            _collision_shape->set_shape(Ref<Shape3D>());
        }
        return;
    }

    if (!_mesh_instance) {
        _mesh_instance = memnew(MeshInstance3D);
        add_child(_mesh_instance);
    }

    _mesh_instance->set_mesh(mesh);

    if (_collision_shape) {
        Ref<Shape3D> shape = mesh->create_trimesh_shape();
        if (shape.is_valid()) {
            _collision_shape->set_shape(shape);
        }
    }
}