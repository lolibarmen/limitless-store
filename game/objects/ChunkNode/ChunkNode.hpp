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
    Ref<PlanetData> planet_data;
    
    StaticBody3D *static_body;
    MeshInstance3D *mesh_instance;
    CollisionShape3D *collision_shape;

    // Инструмент для построения меша
    Ref<SurfaceTool> surface_tool;

    // Вспомогательные методы для генерации поверхности
    float get_density(const Vector3i &p_index) const;
    void build_mesh();
    void create_surface_mesh(int p_size = 6);
    void create_surface_mesh_quad(const Vector3i &p_index);
    void add_quad(const Vector3i &p_index, int p_axis_index);
    void add_reversed_quad(const Vector3i &p_index, int p_axis_index);
    Vector3 get_surface_position(const Vector3i &p_index) const;
    Vector3 get_surface_gradient(const Vector3i &p_index, float p_sample_value) const;
    void add_vertex(const Vector3i &p_index);

protected:
    static void _bind_methods();

public:
    static constexpr float CHUNK_SIZE = 16.0f;
    static constexpr int BLOCKS_IN_CHUNK = 16;
    static constexpr float BLOCK_SIZE = 1.0f;

    ChunkNode();
    ~ChunkNode();

    void _ready() override;

    void set_planet_data(Ref<PlanetData> p_data);
};

} // namespace godot