#include "Player.hpp"
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticWorld/SemanticWorld.hpp>

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

    if (!Engine::get_singleton()->is_editor_hint()) {
        _field_shape.instantiate();
        _field_shape->set_position(get_global_position());
        if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
            sw->register_shape(_field_shape);
        }
    }
}

void Player::_exit_tree() {
    if (_field_shape.is_valid() && _field_shape->get_id() != 0) {
        if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
            sw->unregister_shape_with_owned(_field_shape->get_id());
        }
    }
    _field_shape.unref();
}

void Player::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;
    movement->update_movement(this, delta);
}

void Player::_process(double delta) {
    if (_field_shape.is_valid() && _field_shape->get_id() != 0) {
        _field_shape->set_position(get_global_position());
    }

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