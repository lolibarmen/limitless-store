#include "OuterWildsCharacter.hpp"
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void OuterWildsCharacter::_ready() {
    // Коллизия
    collision_shape = memnew(CollisionShape3D);
    CapsuleShape3D *capsule_shape = memnew(CapsuleShape3D);
    capsule_shape->set_height(1.8f);
    collision_shape->set_shape(capsule_shape);
    add_child(collision_shape);

    // Камера
    camera_pivot = memnew(Node3D);
    camera_pivot->set_position(Vector3(0, 0.8f, 0));
    camera = memnew(Camera3D);
    camera->make_current();
    add_child(camera_pivot);
    camera_pivot->add_child(camera);
}

void OuterWildsCharacter::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    float speed = 2.0f;

    Vector3 velocity = get_velocity();

    if (!is_on_floor()) {
        velocity.y -= gravity * (float)delta;
    }

    Input *input = Input::get_singleton();
    if (input->is_action_just_pressed("ui_accept") && is_on_floor()) {
        velocity.y += 4.5;
    }

    Vector2 input_dir = Vector2(
        input->get_axis("move_left", "move_right"),
        input->get_axis("move_back", "move_forward")
    );

    Vector3 direction = Vector3();
    if (input_dir != Vector2()) {
        Transform3D transform = get_global_transform();
        direction = (transform.basis.get_column(0) * input_dir.x
                   - transform.basis.get_column(2) * input_dir.y).normalized();
    }

    if (direction != Vector3()) {
        velocity.x = direction.x * speed;
        velocity.z = direction.z * speed;
    } else {
        // Плавная остановка
        velocity.x = Math::move_toward(velocity.x, 0.0f, speed);
        velocity.z = Math::move_toward(velocity.z, 0.0f, speed);
    }

    set_velocity(velocity);
    move_and_slide();
}

void OuterWildsCharacter::_input(const Ref<InputEvent> &event) {
    // Обработка движения мыши
    const InputEventMouseMotion *mouse_motion =
        Object::cast_to<InputEventMouseMotion>(event.ptr());

    if (mouse_motion && Input::get_singleton()->get_mouse_mode()
                        == Input::MOUSE_MODE_CAPTURED) {
        float mouse_sensitivity = 0.002f;

        // Горизонтальный поворот — вращаем сам CharacterBody3D
        rotate_y(-mouse_motion->get_relative().x * mouse_sensitivity);

        float camera_pitch = camera_pivot->get_rotation().x;

        // Вертикальный поворот — вращаем camera_pivot, с ограничением
        camera_pitch -= mouse_motion->get_relative().y * mouse_sensitivity;
        camera_pitch = Math::clamp(camera_pitch,
                                   (float)(-Math_PI / 2.0 + 0.1),
                                   (float)( Math_PI / 2.0 - 0.1));
        camera_pivot->set_rotation(Vector3(camera_pitch, 0, 0));
    }

    if (event->is_action_pressed("ui_left_click")) {
        
    }

    if (event->is_action_pressed("ui_right_click")) {
        
    }

    if (event->is_action_pressed("ui_cancel")) {
        Input::MouseMode current_mode = Input::get_singleton()->get_mouse_mode();
        if (current_mode == Input::MOUSE_MODE_CAPTURED) {
            Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
        } else {
            Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
        }
    }
}