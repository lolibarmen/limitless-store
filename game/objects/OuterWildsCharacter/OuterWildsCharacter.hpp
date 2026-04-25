#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>

#include <SuperDigger/SuperDigger.hpp>

namespace godot {

class OuterWildsCharacter : public CharacterBody3D {
    GDCLASS(OuterWildsCharacter, CharacterBody3D)

private:
    CollisionShape3D *collision_shape;
    Camera3D *camera;
    Node3D *camera_pivot;

    float gravity = 9.8f;

    Ref<Tool> current_tool;

    Dictionary perform_raycast(float max_distance = 1000.0f);

protected:
    static void _bind_methods() {};

public:
    OuterWildsCharacter() = default;

    void _ready() override;
    void _physics_process(double delta) override;
    void _input(const Ref<InputEvent> &event) override;
};

} // namespace godot