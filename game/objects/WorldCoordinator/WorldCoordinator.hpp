#pragma once
#include <godot_cpp/classes/node3d.hpp>

namespace godot {

class WorldCoordinator : public Node3D {
    GDCLASS(WorldCoordinator, Node3D)

private:
    int frame_counter = 0;

    void _register_initial_shapes();
    
protected:
    static void _bind_methods();

public:
    WorldCoordinator() = default;
    ~WorldCoordinator() override = default;

    void _ready() override;
    void _process(double delta) override;
};

} // namespace godot