#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <BlockSource/BlockSource.hpp>

namespace godot {

class ChunkNode;

class ChunkMesh : public MeshInstance3D {
    GDCLASS(ChunkMesh, MeshInstance3D)

protected:
    static void _bind_methods() {};

public:
    ChunkMesh() = default;

    void build(Ref<BlockSource> p_source, ChunkNode* p_chunk);
    void build_async(Ref<BlockSource> p_source, ChunkNode* p_chunk);
};

} // namespace godot