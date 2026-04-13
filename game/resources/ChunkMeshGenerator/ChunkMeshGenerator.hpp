#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include <BlockLODSource/BlockLODSource.hpp>

namespace godot {

class ChunkNode;
class ChunkMesh;

class ChunkMeshGenerator : public RefCounted {
    GDCLASS(ChunkMeshGenerator, RefCounted)
    
protected:
    static void _bind_methods() {};

public:
    ChunkMeshGenerator() = default;

    void build(Ref<BlockLODSource> p_source, ChunkNode* p_chunk);
};

} // namespace godot