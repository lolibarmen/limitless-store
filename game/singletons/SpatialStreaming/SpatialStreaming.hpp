#pragma once

#include "Chunk.hpp"
#include <Utils/SpatialHash.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

namespace godot {

class SemanticWorld;

constexpr int   MAX_DEPTH = 2;
constexpr float ROOT_SIZE = 64.0f;
constexpr int   ROOT_RADIUS = 2;

class SpatialStreaming : public Node {
    GDCLASS(SpatialStreaming, Node)

private:
    int root_radius = ROOT_RADIUS;
    
    std::unordered_map<Vector3i, std::unique_ptr<Chunk>, Vector3iHash> roots;
    
    std::unordered_map<Vector3i, Chunk*, Vector3iHash> leaf_chunks;
    
    std::unordered_map<Vector3i, AABB, Vector3iHash> _prev_active_zones;
    
    Vector3 player_pos = {};

    // Внутренняя логика октодерева
    void update_roots();
    void update_recurs(Chunk* n);
    void delete_children(Chunk* n);
    void spawn_chunk(std::unique_ptr<Chunk> chunk_ptr, const Vector3i& key);

    void _notify_changes();

    void _sync_debug_mesh(Chunk* chunk);

protected:
    static void _bind_methods();

public:
    static SpatialStreaming* get_singleton();

    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot