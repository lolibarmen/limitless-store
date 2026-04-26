#pragma once

#include "SurfaceNets.hpp"
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <BlockSource/BlockSource.hpp>
#include <memory>

namespace godot {

class NeochunkNode : public StaticBody3D {
    GDCLASS(NeochunkNode, StaticBody3D)

private:

    static void _build_mesh_task(uint64_t chunk_id);

    int lod_level = 0;
    const int voxel_count = 16;
    float chunk_size = 16.0f;

    Ref<BlockSource> block_source;
    
    MeshInstance3D* mesh_instance;
    CollisionShape3D* collision_shape;

    MeshInstance3D* debug_mesh_instance = nullptr;
    
    void add_debug_box();
    void set_debug_material();
    
protected:
    static void _bind_methods();
    
    public:
    NeochunkNode();
    ~NeochunkNode();
    
    void _ready() override;
    
    void generate_mesh();
    void set_mesh(const Ref<Mesh> mesh);
    void set_lod_level(const int new_lod_level) { lod_level = new_lod_level; }
    void set_chunk_size(const float new_chunk_size) { chunk_size = new_chunk_size; }

    void set_block_source(const Ref<BlockSource> new_block_source) { block_source = new_block_source; }
    Ref<BlockSource> get_block_source() const { return block_source; }

    struct ChunkTaskData {
        std::shared_ptr<ChunkBuildInput> input;
        WorkerThreadPool::TaskID task_id = WorkerThreadPool::INVALID_TASK_ID;
    } _task;
};

} // namespace godot