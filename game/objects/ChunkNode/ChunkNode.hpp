#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

#include <PlanetData/PlanetData.hpp>
#include <ChunkMesh/ChunkMesh.hpp>
#include <ChunkCollider/ChunkCollider.hpp>

namespace godot {

class PlanetNode;

class ChunkNode : public Node3D {
    GDCLASS(ChunkNode, Node3D)

private:

    // === CONFIGURATION ===

    Ref<PlanetData> planet_data;

    Vector3i origin; // начало области в координатах планеты (voxel space)
    int voxel_count = 8; // сколько вокселей генерируем по оси
    int lod = 1; // шаг выборки (LOD)

    // === GODOT NODES ===

    ChunkCollider *chunk_collider = nullptr;
    MeshInstance3D *mesh_instance = nullptr;

    PlanetNode *planet_node = nullptr;
    
    // === MESH GENERATION ===

    Ref<ChunkMesh> chunk_mesh;

    void build_mesh();

    void apply_debug_material();

protected:
    static void _bind_methods();

public:

    ChunkNode();
    ~ChunkNode();

    void _ready() override;

    // 🔥 ЕДИНСТВЕННАЯ настройка чанка
    void configure(
        PlanetNode* p_planet,
        Ref<PlanetData> p_data,
        const Vector3i &p_origin,
        int p_voxel_count,
        int p_sample_step
    );

    const Vector3i& get_origin() const { return origin; }
    void set_origin(const Vector3i& new_origin) { origin = new_origin; }

    int get_voxel_count() const { return voxel_count; }
    void set_voxel_count(int new_count) { voxel_count = new_count; }

    int get_lod() const { return lod; }
    void set_lod(int new_lod) { lod = new_lod; }

    void on_ray_hit(const Dictionary &result);
    void on_ray_enter();
    void on_ray_exit();
    void on_ray_hover(const Vector3 &position);
};

} // namespace godot