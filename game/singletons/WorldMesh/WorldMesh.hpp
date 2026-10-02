#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <cstdint>

namespace godot {

class MeshGenerator;
class Chunk;

class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)
    
private:
    void _remove_mesh_from_chunk(Chunk* chunk, uint64_t shape_id);
    void _clear_shape_in_subtree(Chunk* node, uint64_t shape_id);

protected:
    static void _bind_methods();

public:
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    void request_render(uint64_t shape_id, uint64_t chunk_id, Ref<MeshGenerator> generator);
    void cancel_render(uint64_t shape_id, uint64_t chunk_id);
    void complete_mesh(uint64_t shape_id, uint64_t chunk_id, Ref<ArrayMesh> mesh);
};

} // namespace godot