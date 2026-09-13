#pragma once

#include <ChunkNode/ChunkNode.hpp>
#include <Utils/SpatialHash.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace godot {

constexpr int   MAX_DEPTH = 2;
constexpr float ROOT_SIZE = 64.0f;
constexpr int   ROOT_RADIUS = 8;

class WorldMesh : public Node3D {
    GDCLASS(WorldMesh, Node3D)

    int root_radius = ROOT_RADIUS;
    std::unordered_map<Vector3i, ChunkNode*, Vector3iHash> roots;
    Vector3 player_pos = {};
    
    std::unordered_set<uint64_t> _registered_shape_ids;

    void update_roots();
    void update_recurs(ChunkNode* n);
    void delete_children(ChunkNode* n);

    void spawn_chunk(ChunkNode* n);
    void despawn_chunk(ChunkNode* n);
    
    ChunkNode* find_leaf_chunk(const Vector3& pos) const;

protected:
    static void _bind_methods();

public:
    static WorldMesh* get_singleton();

    void _ready() override;
    void _process(double delta) override;

    void refresh_chunks_in_aabb(const AABB& bounds);

    void register_shape(uint64_t id);
    void unregister_shape(uint64_t id);
    void update_mesh(uint64_t id);

    void complete_mesh(uint64_t id, Ref<ArrayMesh> mesh);
};

} // namespace godot