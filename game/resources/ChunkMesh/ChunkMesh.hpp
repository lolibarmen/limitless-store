#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <PlanetData/PlanetData.hpp>

namespace godot {

class ChunkNode;

class ChunkMesh : public MeshInstance3D {
    GDCLASS(ChunkMesh, MeshInstance3D)

protected:
    static void _bind_methods();

public:
    ChunkMesh() = default;

    void build(Ref<PlanetData> p_data, ChunkNode* p_chunk); 
};

} // namespace godot