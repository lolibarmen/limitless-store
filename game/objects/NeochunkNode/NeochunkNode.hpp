#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <ChunkMeshNode/ChunkMeshNode.hpp>

namespace godot {

class NeochunkNode : public Node3D {
    GDCLASS(NeochunkNode, Node3D)

private:
    int _lod_level = 0;
    float _chunk_size = 16.0f;
    float _voxel_count = 16.0f;

    ChunkMeshNode* _mesh_node = nullptr;

protected:
    static void _bind_methods();

public:
    NeochunkNode();
    ~NeochunkNode() override;

    void _ready() override;
    void generate();

    void set_lod_level(int level);
    void set_chunk_size(float size);
    void set_voxel_count(float count);
    
    int get_lod_level() const { return _lod_level; }
    float get_chunk_size() const { return _chunk_size; }
    float get_voxel_count() const { return _voxel_count; }
};

} // namespace godot