#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

#include <PlanetData/PlanetData.hpp>

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

    Ref<SurfaceTool> surface_tool;


    // === MESH GENERATION ===

    float get_density(const Vector3i &p_index) const;

    void build_mesh();
    void create_surface_mesh(int p_size);
    void create_surface_mesh_quad(const Vector3i &p_index);

    void add_quad(const Vector3i &p_index, int p_axis_index);
    void add_reversed_quad(const Vector3i &p_index, int p_axis_index);

    Vector3 get_surface_position(const Vector3i &p_index) const;
    Vector3 get_surface_gradient(const Vector3i &p_index, float p_sample_value) const;

    void add_vertex(const Vector3i &p_index);

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
};

} // namespace godot