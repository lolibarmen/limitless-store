#pragma once
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <resources/PlanetData/PlanetData.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

namespace godot {

class ChunkNode;

class ChunkMesh : public MeshInstance3D {
    GDCLASS(ChunkMesh, MeshInstance3D)

protected:
    static void _bind_methods();

public:
    ChunkMesh() = default;

    // Строит и сразу применяет меш к себе
    void build(ChunkNode* p_chunk, Ref<PlanetData> p_data);
};

} // namespace godot