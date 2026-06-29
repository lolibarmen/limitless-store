#include "PlayerMovement.hpp"
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

PlayerMovement::PlayerMovement() {}

void PlayerMovement::_bind_methods() {}

void PlayerMovement::update_movement(CharacterBody3D* player, double delta) {
    if (!player || Engine::get_singleton()->is_editor_hint()) return;

    float speed = walk_speed;
    Vector3 velocity = player->get_velocity();

    if (!player->is_on_floor()) {
        velocity.y -= gravity * (float)delta;
    }

    Input* input = Input::get_singleton();
    if (input->is_action_just_pressed("ui_accept")) {
        velocity.y += 4.5f;
    }
    if (input->is_action_pressed("run")) {
        speed = run_speed;
    }

    Vector2 input_dir = Vector2(
        input->get_axis("move_left", "move_right"),
        input->get_axis("move_back", "move_forward")
    );

    Vector3 direction;
    if (input_dir != Vector2()) {
        Transform3D transform = player->get_global_transform();
        direction = (transform.basis.get_column(0) * input_dir.x
                   - transform.basis.get_column(2) * input_dir.y).normalized();
    }

    if (direction != Vector3()) {
        velocity.x = direction.x * speed;
        velocity.z = direction.z * speed;
    } else {
        velocity.x = Math::move_toward(velocity.x, 0.0f, speed);
        velocity.z = Math::move_toward(velocity.z, 0.0f, speed);
    }

    player->set_velocity(velocity);
    player->move_and_slide();
}