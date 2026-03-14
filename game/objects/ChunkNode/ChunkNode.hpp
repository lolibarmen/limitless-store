#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <PlanetData/PlanetData.hpp>
#include <ChunkMesh/ChunkMesh.hpp>
#include <ChunkCollider/ChunkCollider.hpp>

namespace godot {

class ChunkManager;

class ChunkNode : public Node3D {
    GDCLASS(ChunkNode, Node3D)

private:
    // === КОНФИГУРАЦИЯ ===
    Ref<PlanetData> planet_data;
    Vector3i        origin;
    int             voxel_count = 8;
    int             lod         = 1;

    // === GODOT NODES ===
    ChunkMesh*      chunk_mesh     = nullptr;
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

    void configure(
        ChunkManager*   p_manager,
        Ref<PlanetData> p_data,
        const Vector3i& p_origin,
        int             p_voxel_count,
        int             p_lod
    );

    const Vector3i& get_origin()      const { return origin; }
    int             get_voxel_count() const { return voxel_count; }
    int             get_lod()         const { return lod; }

    int  get_neighbor_lod(const Vector3i& direction) const;

    void build_mesh();
    void action(const Vector3& world_pos, float radius, float delta);
};

} // namespace godot