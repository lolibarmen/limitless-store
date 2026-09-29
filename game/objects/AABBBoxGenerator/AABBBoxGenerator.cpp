#include "AABBBoxGenerator.hpp"
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void AABBBoxGenerator::_bind_methods() {}

Ref<ArrayMesh> AABBBoxGenerator::generate(uint64_t shape_id, const AABB& bounds) const {
    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // Половинные размеры для построения бокса вокруг центра (0,0,0)
    Vector3 h = bounds.size / 2.0f;

    // 8 углов бокса
    Vector3 p[8] = {
        {-h.x, -h.y, -h.z}, { h.x, -h.y, -h.z}, { h.x,  h.y, -h.z}, {-h.x,  h.y, -h.z}, // Back
        {-h.x, -h.y,  h.z}, { h.x, -h.y,  h.z}, { h.x,  h.y,  h.z}, {-h.x,  h.y,  h.z}  // Front
    };

    // Front (z+)
    st->add_vertex(p[4]); st->add_vertex(p[5]); st->add_vertex(p[6]);
    st->add_vertex(p[4]); st->add_vertex(p[6]); st->add_vertex(p[7]);
    // Back (z-)
    st->add_vertex(p[1]); st->add_vertex(p[0]); st->add_vertex(p[3]);
    st->add_vertex(p[1]); st->add_vertex(p[3]); st->add_vertex(p[2]);
    // Left (x-)
    st->add_vertex(p[0]); st->add_vertex(p[4]); st->add_vertex(p[7]);
    st->add_vertex(p[0]); st->add_vertex(p[7]); st->add_vertex(p[3]);
    // Right (x+)
    st->add_vertex(p[5]); st->add_vertex(p[1]); st->add_vertex(p[2]);
    st->add_vertex(p[5]); st->add_vertex(p[2]); st->add_vertex(p[6]);
    // Top (y+)
    st->add_vertex(p[7]); st->add_vertex(p[6]); st->add_vertex(p[2]);
    st->add_vertex(p[7]); st->add_vertex(p[2]); st->add_vertex(p[3]);
    // Bottom (y-)
    st->add_vertex(p[0]); st->add_vertex(p[1]); st->add_vertex(p[5]);
    st->add_vertex(p[0]); st->add_vertex(p[5]); st->add_vertex(p[4]);

    st->generate_normals();
    
    return st->commit();
}