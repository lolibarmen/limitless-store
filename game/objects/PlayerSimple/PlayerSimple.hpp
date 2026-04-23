#pragma once
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot {

class PlayerSimple : public CharacterBody3D {
    GDCLASS(PlayerSimple, CharacterBody3D)

private:
    MeshInstance3D *mesh_instance;
    CollisionShape3D *collision_shape;
    Camera3D *camera;
    Node3D *camera_pivot; // вертикальный поворот (pitch)

    float speed = 5.0f;
    float jump_velocity = 4.5f;
    float mouse_sensitivity = 0.002f;
    float gravity = 9.8f;

    float camera_pitch = 0.0f; // ограничение угла камеры вверх/вниз
    
protected:
    static void _bind_methods();

public:
    PlayerSimple() = default;

    void _ready() override;
    void _physics_process(double delta) override;
    void _input(const Ref<InputEvent> &event) override;
};

} // namespace godot