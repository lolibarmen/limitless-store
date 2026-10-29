#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>

namespace godot {

class SpatialStreaming : public Node {
    GDCLASS(SpatialStreaming, Node)

private:
    Vector3 player_pos = {};
    // Глубина, при которой лист октодерева становится минимальным чанком 16 м:
    // корень 4096 м / 2^8 = 16 м (16 точек с шагом 1 м на ребро).
    int max_depth = 8; // Настраиваемая глубина детализации

protected:
    static void _bind_methods();

public:
    static SpatialStreaming* get_singleton();
    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot