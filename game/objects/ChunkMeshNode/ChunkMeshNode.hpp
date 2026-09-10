#pragma once

#include "SurfaceNets.hpp"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <memory>

namespace godot {

struct ChunkMeshTask {
    std::shared_ptr<ChunkBuildInput> input;
    uint64_t task_id;
};

class ChunkMeshNode : public Node3D {
    GDCLASS(ChunkMeshNode, Node3D)

private:
    MeshInstance3D* _mesh_instance = nullptr;
    CollisionShape3D* _collision_shape = nullptr;
    ChunkMeshTask _task;

    static void _build_mesh_task(uint64_t node_id, uint64_t world_id);

protected:
    static void _bind_methods();

public:
    ChunkMeshNode();
    ~ChunkMeshNode() override;
    void _ready() override;

    void generate_mesh(uint64_t world_id);
    void set_mesh(Ref<Mesh> mesh);
    
    void set_lod_level(int level) { _lod_level = level; }
    void set_chunk_size(float size) { _chunk_size = size; }
    void set_voxel_count(float count) { _voxel_count = count; }

private:
    int _lod_level = 0;
    float _chunk_size = 16.0f;
    float _voxel_count = 16.0f;
};

} // namespace godot