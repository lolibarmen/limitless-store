#include "NeochunkNode.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void NeochunkNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_lod_level", "level"), &NeochunkNode::set_lod_level);
    ClassDB::bind_method(D_METHOD("set_chunk_size", "size"), &NeochunkNode::set_chunk_size);
}

NeochunkNode::NeochunkNode() {}
NeochunkNode::~NeochunkNode() {}

void NeochunkNode::_ready() {
    add_to_group("mineable");
    _mesh_node = memnew(ChunkMeshNode);
    add_child(_mesh_node);

    _mesh_node->set_lod_level(_lod_level);
    _mesh_node->set_chunk_size(_chunk_size);
    _mesh_node->set_voxel_count(_voxel_count);
}

void NeochunkNode::generate() {
    if (_mesh_node) {
        _mesh_node->generate_mesh();
    }
}

void NeochunkNode::set_lod_level(int level) {
    _lod_level = level;
    if (_mesh_node) _mesh_node->set_lod_level(level);
}

void NeochunkNode::set_chunk_size(float size) {
    _chunk_size = size;
    if (_mesh_node) _mesh_node->set_chunk_size(size);
}

void NeochunkNode::set_voxel_count(float count) {
    _voxel_count = count;
    if (_mesh_node) _mesh_node->set_voxel_count(count);
}