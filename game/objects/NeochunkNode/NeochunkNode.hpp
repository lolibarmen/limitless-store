#pragma once

#include "SurfaceNets.hpp"
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <BlockLODSource/BlockLODSource.hpp>
#include <memory>

namespace godot {

class NeochunkNode : public StaticBody3D {
    GDCLASS(NeochunkNode, StaticBody3D)

private:

    struct ChunkTaskData {
        std::shared_ptr<ChunkBuildInput> input;
        uint64_t chunk_id;
        WorkerThreadPool::TaskID task_id = WorkerThreadPool::INVALID_TASK_ID;
    } _task;
    void _build_mesh_task();

    int lod_level = 0;
    const int voxel_count = 16;
    float chunk_size = 16.0f;

    Ref<BlockLODSource> block_source;
    
    MeshInstance3D* mesh_instance;
    CollisionShape3D* collision_shape;

    void add_debug_box();
    void set_debug_material();

    void generate_mesh();

protected:
    static void _bind_methods();

public:
    NeochunkNode();
    ~NeochunkNode();

    void _ready() override;
    // void _exit_tree() override { print_line(" - realy despawn"); }

    void set_mesh(const Ref<Mesh> mesh);
    void set_block_source(const Ref<BlockLODSource> new_block_source) { block_source = new_block_source; }
    void set_lod_level(const int new_lod_level) { lod_level = new_lod_level; }
    void set_chunk_size(const float new_chunk_size) { chunk_size = new_chunk_size; }
};

} // namespace godot