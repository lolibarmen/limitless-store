#pragma once
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

namespace godot {

class SpatialStreaming : public Node {
    GDCLASS(SpatialStreaming, Node)

private:
    Vector3 player_pos = {};
    int max_depth = 3; // Настраиваемая глубина детализации
    
    // Храним предыдущие активные зоны для каждой фигуры отдельно
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> _prev_active_zones;

    void _notify_changes();

protected:
    static void _bind_methods();

public:
    static SpatialStreaming* get_singleton();
    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot