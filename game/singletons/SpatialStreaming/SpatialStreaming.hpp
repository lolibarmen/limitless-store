#pragma once
#include <SemanticWorld/SemanticWorld.hpp>
#include <Utils/SpatialHash.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <unordered_map>
#include <vector>

namespace godot {

class Chunk;

class SpatialStreaming : public Node {
    GDCLASS(SpatialStreaming, Node)

private:
    std::unordered_map<Vector3i, AABB, Vector3iHash> _prev_active_zones;
    
    Vector3 player_pos = {};
    int root_radius = 2;
    
    // Константы для логики сплита/мерджа
    static constexpr float ROOT_SIZE = 64.0f;
    static constexpr int MAX_DEPTH = 2;

    // Логика принятия решений
    void update_root_zones();
    void evaluate_and_update_tree(Chunk* node);
    
    // Уведомление внешнего мира
    void _notify_changes();

protected:
    static void _bind_methods();

public:
    static SpatialStreaming* get_singleton();

    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot