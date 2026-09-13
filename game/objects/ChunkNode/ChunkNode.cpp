#include "ChunkNode.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ChunkNode::ChunkNode(Vector3 c, float s, int d, ChunkNode* p)
    : center(c), size(s), depth(d), parent(p) {}

ChunkNode::~ChunkNode() {
    mesh_instances.clear();
}

void ChunkNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_lod_level", "level"), &ChunkNode::set_lod_level);
    ClassDB::bind_method(D_METHOD("get_lod_level"), &ChunkNode::get_lod_level);
    ClassDB::add_property("ChunkNode", PropertyInfo(Variant::INT, "lod_level"), "set_lod_level", "get_lod_level");

    ClassDB::bind_method(D_METHOD("set_chunk_size", "size"), &ChunkNode::set_chunk_size);
    ClassDB::bind_method(D_METHOD("get_chunk_size"), &ChunkNode::get_chunk_size);
    ClassDB::add_property("ChunkNode", PropertyInfo(Variant::FLOAT, "chunk_size"), "set_chunk_size", "get_chunk_size");

    ClassDB::bind_method(D_METHOD("set_voxel_count", "count"), &ChunkNode::set_voxel_count);
    ClassDB::bind_method(D_METHOD("get_voxel_count"), &ChunkNode::get_voxel_count);
    ClassDB::add_property("ChunkNode", PropertyInfo(Variant::FLOAT, "voxel_count"), "set_voxel_count", "get_voxel_count");

    ClassDB::bind_method(D_METHOD("set_show_bounds", "show"), &ChunkNode::set_show_bounds);
    ClassDB::bind_method(D_METHOD("get_show_bounds"), &ChunkNode::get_show_bounds);
    ClassDB::add_property("ChunkNode", PropertyInfo(Variant::BOOL, "show_bounds", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_show_bounds", "get_show_bounds");
}

void ChunkNode::_ready() {
}

void ChunkNode::generate() {
}

void ChunkNode::add_mesh_instance(MeshInstance3D* mesh) {
    if (mesh) {
        mesh_instances.push_back(mesh);
        add_child(mesh);
    }
}

void ChunkNode::clear_meshes() {
    for (auto* mesh : mesh_instances) {
        if (mesh) {
            mesh->queue_free();
        }
    }
    mesh_instances.clear();
}

void ChunkNode::set_lod_level(int level) { 
    _lod_level = level; 
}
int ChunkNode::get_lod_level() const { 
    return _lod_level; 
}

void ChunkNode::set_chunk_size(float size) { 
    _chunk_size = size; 
    _update_bounds_mesh(); 
}
float ChunkNode::get_chunk_size() const { 
    return _chunk_size; 
}

void ChunkNode::set_voxel_count(float count) { 
    _voxel_count = count; 
}
float ChunkNode::get_voxel_count() const { 
    return _voxel_count; 
}

void ChunkNode::set_show_bounds(bool show) { 
    _show_bounds = show; 
    _update_bounds_mesh(); 
}
bool ChunkNode::get_show_bounds() const { 
    return _show_bounds; 
}

void ChunkNode::_update_bounds_mesh() {
    // Пока пустой
}