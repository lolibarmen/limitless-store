#pragma once
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Минимальный дескриптор планеты.
// Передаётся от SolarSystemManager → PlanetManager.
struct PlanetContext {
    int     seed   = 0;
    Vector3 position;
    float   radius = 100.0f;
};

} // namespace godot