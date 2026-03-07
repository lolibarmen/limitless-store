#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

#include <PlanetData/PlanetData.hpp>

namespace godot {

class ChunkNode;

class ChunkMesh : public Resource {
    GDCLASS(ChunkMesh, Resource)

private:

    ChunkNode* chunk;
    Ref<PlanetData> planet_data;

    Ref<SurfaceTool> st;

    void create_surface_mesh(int p_size);
    void create_surface_mesh_quad(const Vector3i &p_index);

    void add_quad(const Vector3i &p_index, int p_axis_index);
    void add_reversed_quad(const Vector3i &p_index, int p_axis_index);

    Vector3 get_surface_position(const Vector3i &p_index) const;
    Vector3 get_surface_gradient(const Vector3i &p_index, float p_sample_value) const;

    void add_vertex(const Vector3i &p_index);


    float get_density(const Vector3i &p_index) const;

protected:
    static void _bind_methods();

public:
    ChunkMesh();
    ~ChunkMesh();

    Ref<ArrayMesh> build(Ref<PlanetData> p_data, ChunkNode* p_chunk); 
};

} // namespace godot