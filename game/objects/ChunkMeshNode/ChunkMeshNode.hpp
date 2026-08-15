#pragma once

#include "SurfaceNets.hpp"
#include "ChunkMeshQueue.hpp"
#include "ChunkMaterialManager.hpp"
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <BlockSource/BlockSource.hpp>
#include <memory>

namespace godot {

class ChunkMeshNode : public StaticBody3D {
    GDCLASS(ChunkMeshNode, StaticBody3D)

private:
    MeshInstance3D* _mesh_instance = nullptr;
    CollisionShape3D* _collision_shape = nullptr;

    int _lod_level = 0;
    const int _voxel_count = 16;
    float _chunk_size = 16.0f;

    Ref<BlockSource> _block_source;

    struct ChunkTaskData {
        std::shared_ptr<ChunkBuildInput> input;
        WorkerThreadPool::TaskID task_id = WorkerThreadPool::INVALID_TASK_ID;
    } _task;

    static void _build_mesh_task(uint64_t node_id);

    void set_debug_material();

protected:
    static void _bind_methods();

public:
    ChunkMeshNode();
    ~ChunkMeshNode();

    void _ready() override;

    void generate_mesh();
    void set_mesh(Ref<Mesh> mesh);

    // Сеттеры для проброса параметров от родителя
    void set_block_source(Ref<BlockSource> source) { _block_source = source; }
    void set_lod_level(int level) { _lod_level = level; }
    void set_chunk_size(float size) { _chunk_size = size; }

    Ref<BlockSource> get_block_source() const { return _block_source; }
};

} // namespace godot