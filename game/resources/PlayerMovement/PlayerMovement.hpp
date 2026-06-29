#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/character_body3d.hpp>

namespace godot {

class PlayerMovement : public RefCounted {
    GDCLASS(PlayerMovement, RefCounted)

private:
    float walk_speed = 2.1f;
    float run_speed = 16.5f;
    float gravity = 9.8f;

protected:
    static void _bind_methods();

public:
    PlayerMovement();
    void update_movement(CharacterBody3D* player, double delta);
};

}