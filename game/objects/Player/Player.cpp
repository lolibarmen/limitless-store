#include "Player.hpp"
#include <SuperDigger/SuperDigger.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void Player::_ready() {
    collision_shape = memnew(CollisionShape3D);
    CapsuleShape3D* capsule = memnew(CapsuleShape3D);
    capsule->set_height(1.8f);
    collision_shape->set_shape(capsule);
    add_child(collision_shape);

    movement.instantiate();
    camera_ctl.instantiate();
    interaction.instantiate();

    // Создаём ToolManager как дочерний узел
    tool_mgr = memnew(ToolManager);
    add_child(tool_mgr);

    camera_ctl->setup_camera(this);

    // Начальный инструмент
    SuperDigger* digger = memnew(SuperDigger);
    tool_mgr->set_tool(digger);
}

void Player::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;
    movement->update_movement(this, delta);
}

void Player::_process(double delta) {
    Camera3D* cam = camera_ctl->get_camera();
    Dictionary ray = interaction->perform_raycast(cam, this);
    tool_mgr->update_tool(ray);
}

void Player::_input(const Ref<InputEvent>& event) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    camera_ctl->process_input(this, event);

    Camera3D* cam = camera_ctl->get_camera();
    interaction->process_input(cam, tool_mgr, event);
}