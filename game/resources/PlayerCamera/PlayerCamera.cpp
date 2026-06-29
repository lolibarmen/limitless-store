#include "PlayerCamera.hpp"
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

PlayerCamera::PlayerCamera() {}

void PlayerCamera::_bind_methods() {}

void PlayerCamera::setup_camera(Node3D* parent) {
    if (!parent) return;

    camera_pivot = memnew(Node3D);
    camera_pivot->set_position(Vector3(0, 0.8f, 0));
    parent->add_child(camera_pivot);

    camera = memnew(Camera3D);
    camera->set_far(10000);
    camera->make_current();
    camera_pivot->add_child(camera);

    Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
}

void PlayerCamera::process_input(Node3D* parent, const Ref<InputEvent>& event) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    const InputEventMouseMotion* mouse_motion =
        Object::cast_to<InputEventMouseMotion>(event.ptr());

    if (mouse_motion && Input::get_singleton()->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
        if (parent) {
            parent->rotate_y(-mouse_motion->get_relative().x * sensitivity);
        }

        if (camera_pivot) {
            float pitch = camera_pivot->get_rotation().x;
            pitch -= mouse_motion->get_relative().y * sensitivity;
            pitch = Math::clamp(pitch, pitch_min, pitch_max);
            camera_pivot->set_rotation(Vector3(pitch, 0, 0));
        }
    }

    if (event->is_action_pressed("ui_cancel")) {
        toggle_mouse_capture();
    }
}

void PlayerCamera::toggle_mouse_capture() {
    Input::MouseMode mode = Input::get_singleton()->get_mouse_mode();
    Input::get_singleton()->set_mouse_mode(
        (mode == Input::MOUSE_MODE_CAPTURED) ? Input::MOUSE_MODE_VISIBLE
                                             : Input::MOUSE_MODE_CAPTURED
    );
}

Camera3D* PlayerCamera::get_camera() const {
    return camera;
}

Node3D* PlayerCamera::get_pivot() const {
    return camera_pivot;
}