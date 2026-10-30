#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <cstdint>

namespace godot {

class SpatialStreaming : public Node {
    GDCLASS(SpatialStreaming, Node)

private:
    Vector3 player_pos = {};
    // Глобальных параметров детализации больше нет: каждое дерево получает свои
    // дистанции перехода между уровнями (split_distances) при создании через
    // ShapeSpatialManager::create_octree().

protected:
    static void _bind_methods();

public:
    static SpatialStreaming* get_singleton();
    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot