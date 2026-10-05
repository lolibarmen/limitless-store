#pragma once
#include <SemanticWorld/SemanticWorld.hpp>
#include <Utils/SpatialHash.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>

namespace godot {

class Chunk;

class SpatialStreaming : public Node {
    GDCLASS(SpatialStreaming, Node)

private:
    // Храним соответствие: Координаты ячейки -> ID чанка в ChunkOctree
    std::unordered_map<Vector3i, uint64_t, Vector3iHash> _active_roots;
    std::unordered_set<uint64_t> _prev_active_zones;
    
    Vector3 player_pos = {};
    int root_radius = 9;
    
    static constexpr float ROOT_SIZE = 64.0f;
    static constexpr int MAX_DEPTH = 2;

    Vector3i get_cell_from_pos(const Vector3& pos) const;

    void update_root_zones();
    void evaluate_and_update_tree(Chunk* node);
    void _notify_changes();

protected:
    static void _bind_methods();

public:
    static SpatialStreaming* get_singleton();

    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot