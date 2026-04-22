#include "PlayerSimple.hpp"
#include <godot_cpp/classes/capsule_mesh.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void PlayerSimple::_bind_methods() {}

void PlayerSimple::_ready() {
    // Меш
    mesh_instance = memnew(MeshInstance3D);
    CapsuleMesh *capsule_mesh = memnew(CapsuleMesh);
    mesh_instance->set_mesh(capsule_mesh);
    add_child(mesh_instance);

    // Коллизия
    collision_shape = memnew(CollisionShape3D);
    CapsuleShape3D *capsule_shape = memnew(CapsuleShape3D);
    collision_shape->set_shape(capsule_shape);
    add_child(collision_shape);

    // Пивот камеры — дочерний Node3D на уровне головы (y = 1.0)
    camera_pivot = memnew(Node3D);
    camera_pivot->set_position(Vector3(0, 1.0f, 0));
    add_child(camera_pivot);

    // Камера — дочерний объект пивота
    camera = memnew(Camera3D);
    camera_pivot->add_child(camera);
    camera->make_current();

    // Захват мыши
    Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
}

void PlayerSimple::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    Vector3 velocity = get_velocity();

    // Гравитация
    if (!is_on_floor()) {
        velocity.y -= gravity * (float)delta;
    }

    // Прыжок
    Input *input = Input::get_singleton();
    if (input->is_action_just_pressed("ui_accept") && is_on_floor()) {
        velocity.y = jump_velocity;
    }

    // Направление движения (WASD / стрелки)
    Vector2 input_dir = Vector2(
        input->get_axis("move_left", "move_right"),
        input->get_axis("move_back", "move_forward")
    );

    // Движение относительно поворота персонажа
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

void PlayerSimple::_input(const Ref<InputEvent> &event) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Обработка движения мыши
    const InputEventMouseMotion *mouse_motion =
        Object::cast_to<InputEventMouseMotion>(event.ptr());

    if (mouse_motion && Input::get_singleton()->get_mouse_mode()
                        == Input::MOUSE_MODE_CAPTURED) {

        // Горизонтальный поворот — вращаем сам CharacterBody3D
        rotate_y(-mouse_motion->get_relative().x * mouse_sensitivity);

        // Вертикальный поворот — вращаем camera_pivot, с ограничением
        camera_pitch -= mouse_motion->get_relative().y * mouse_sensitivity;
        camera_pitch = Math::clamp(camera_pitch,
                                   (float)(-Math_PI / 2.0 + 0.1),
                                   (float)( Math_PI / 2.0 - 0.1));
        camera_pivot->set_rotation(Vector3(camera_pitch, 0, 0));
    }

    // Выход из захвата мыши по Escape
    if (event->is_action_pressed("ui_cancel")) {
        Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
    }
}