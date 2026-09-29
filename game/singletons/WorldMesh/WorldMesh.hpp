#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace godot {

class MeshGenerator;

class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)
    
    std::unordered_map<uint64_t, std::vector<MeshInstance3D*>> _shape_meshes;

    void _cleanup_shape_meshes(uint64_t shape_id);

protected:
    static void _bind_methods();

public:
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    void request_render(uint64_t shape_id, const AABB& bounds);
    void request_render(uint64_t shape_id, const AABB& bounds, Ref<MeshGenerator> generator);

    void cancel_render(uint64_t shape_id, const AABB& bounds);

    void complete_mesh(uint64_t shape_id, const AABB& bounds, Ref<ArrayMesh> mesh);
};

} // namespace godot