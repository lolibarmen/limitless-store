#include "PlayerMovement.hpp"
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

PlayerMovement::PlayerMovement() {}

void PlayerMovement::_bind_methods() {}

void PlayerMovement::update_movement(CharacterBody3D* player, double delta) {
    if (!player || Engine::get_singleton()->is_editor_hint()) return;

    const float speed = Input::get_singleton()->is_action_pressed("run") ? run_speed : walk_speed;
    Vector3 velocity = player->get_velocity();
    Vector3 up = player->get_up_direction();
    if (up.length_squared() < 0.0001f) {
        up = Vector3(0.0f, 1.0f, 0.0f);
    }
    up.normalize();

    const float vertical_speed = velocity.dot(up);
    Vector3 planar_velocity = velocity - up * vertical_speed;
    float new_vertical_speed = vertical_speed;

    new_vertical_speed -= gravity * (float)delta;

    Input* input = Input::get_singleton();
    if (input->is_action_just_pressed("ui_accept") && player->is_on_floor()) {
        new_vertical_speed = 4.5f;
    }

    Vector2 input_dir = Vector2(
        input->get_axis("move_left", "move_right"),
        input->get_axis("move_back", "move_forward")
    );

    Vector3 direction;
    if (input_dir != Vector2()) {
        Basis basis = player->get_global_basis().orthonormalized();
        Vector3 right = basis.get_column(0);
        Vector3 forward = -basis.get_column(2);
        right -= up * right.dot(up);
        forward -= up * forward.dot(up);
        if (right.length_squared() > 0.0001f) right.normalize();
        if (forward.length_squared() > 0.0001f) forward.normalize();
        direction = (right * input_dir.x + forward * input_dir.y);
        if (direction.length_squared() > 0.0001f) direction.normalize();
    }

    if (direction != Vector3()) {
        planar_velocity = direction * speed;
    } else {
        const float planar_speed = planar_velocity.length();
        const float new_planar_speed = Math::move_toward(planar_speed, 0.0f, speed);
        if (planar_speed > 0.0001f) {
            planar_velocity = planar_velocity * (new_planar_speed / planar_speed);
        } else {
            planar_velocity = Vector3();
        }
    }

    player->set_velocity(planar_velocity + up * new_vertical_speed);
    player->move_and_slide();
}