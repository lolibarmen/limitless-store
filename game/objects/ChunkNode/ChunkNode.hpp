#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

#include <PlanetData/PlanetData.hpp>
#include <ChunkMesh/ChunkMesh.hpp>

namespace godot {

class ChunkNode : public Node3D {
    GDCLASS(ChunkNode, Node3D)

private:

    // === CONFIGURATION ===

    Ref<PlanetData> planet_data;

    // начало области в координатах планеты (voxel space)
    Vector3i origin;

    // сколько вокселей генерируем по оси
    int voxel_count = 8;

    // шаг выборки (LOD)
    int sample_step = 1;


    // === GODOT NODES ===

    StaticBody3D *static_body = nullptr;
    MeshInstance3D *mesh_instance = nullptr;
    CollisionShape3D *collision_shape = nullptr;

    Ref<ChunkMesh> chunk_mesh;


    // === MESH GENERATION ===

    void build_mesh();

protected:
    static void _bind_methods();

public:

    ChunkNode();
    ~ChunkNode();

    void _ready() override;

    // 🔥 ЕДИНСТВЕННАЯ настройка чанка
    void configure(
        Ref<PlanetData> p_data,
        const Vector3i &p_origin,
        int p_voxel_count,
        int p_sample_step
    );

    const Vector3i& get_origin() const { return origin; }
    // void set_origin(const Vector3i& new_origin) { origin = new_origin; }

    int get_voxel_count() const { return voxel_count; }
    // void set_voxel_count(int new_count) { voxel_count = new_count; }

    int get_sample_step() const { return sample_step; }
    // void set_sample_step(int new_step) { sample_step = new_step; }
};

} // namespace godot