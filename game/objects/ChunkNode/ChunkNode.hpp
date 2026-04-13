#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <ChunkCollider/ChunkCollider.hpp>

#include <BlockLODSource/BlockLODSource.hpp>

namespace godot {

class ChunkManager;

class ChunkNode : public Node3D {
    GDCLASS(ChunkNode, Node3D)

private:
    // === КОНФИГУРАЦИЯ ===
    Ref<BlockLODSource> block_source;
    Vector3i            origin;
    int                 voxel_count = 8;
    int                 lod         = 1;

    // === GODOT NODES ===
    MeshInstance3D* chunk_mesh     = nullptr;
    ChunkCollider*  chunk_collider = nullptr;

    // === КУРАТОР ===
    ChunkManager* chunk_manager = nullptr;

    void apply_material();

protected:
    static void _bind_methods();

public:
    ChunkNode();
    ~ChunkNode();

    void _ready() override;

    void init(
        ChunkManager*       p_manager,
        Ref<BlockLODSource> p_source,
        const Vector3i&     p_origin,
        int                 p_voxel_count,
        int                 p_lod
    );

    const Vector3i& get_origin()      const { return origin; }
    int             get_voxel_count() const { return voxel_count; }
    int             get_lod()         const { return lod; }

    int  get_neighbor_lod(const Vector3i& direction) const;

    void build_mesh();

    void set_mesh(Ref<ArrayMesh> p_mesh);
    void trans_metter(const Vector3& world_pos, float delta, float radius);

    ChunkManager* get_chunk_manager() const { return chunk_manager; }
};

} // namespace godot