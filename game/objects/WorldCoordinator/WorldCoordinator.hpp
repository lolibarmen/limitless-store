#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <NeochunkManager/NeochunkManager.hpp>

namespace godot {

class WorldCoordinator : public Node3D {
    GDCLASS(WorldCoordinator, Node3D)

private:
    int seed = 1;

    NeochunkManager*  chunk_manager  = nullptr;
    
protected:
    static void _bind_methods();

public:
    WorldCoordinator() = default;
    ~WorldCoordinator() override = default;

    void _ready() override;

    int  get_seed() const { return seed; }
    void set_seed(int v)  { seed = v; }
};

} // namespace godot