#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <managers/PlanetManager/PlanetManager.hpp>

namespace godot {

class WorldManager : public Node3D {
    GDCLASS(WorldManager, Node3D)

private:
    static WorldManager* singleton;

    PlanetManager* planet_manager = nullptr;

protected:
    static void _bind_methods() {};

public:
    static WorldManager* get_singleton() { return singleton; }

    // void _ready()               override;
    // void _exit_tree()           override;
    // void _process(double delta) override;

    // Прямой вызов — никаких сигналов
    void action(const Vector3& world_pos, float radius, float delta);
};

} // namespace godot