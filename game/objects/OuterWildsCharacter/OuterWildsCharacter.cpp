#include "OuterWildsCharacter.hpp"
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

#include <BuildTool/BuildTool.hpp>

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
    camera->set_far(10000);
    camera->make_current();
    add_child(camera_pivot);
    camera_pivot->add_child(camera);

    // Перключаем мышь
    Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);

    // Интрумент
    BuildTool* build_tool = memnew(BuildTool);
    Ref<PackedScene> wall_scene = ResourceLoader::get_singleton()->load("res://objects/build/Wall.tscn");
    build_tool->set_build_scene(wall_scene);
    add_child(build_tool);
    current_tool = build_tool;
}

void OuterWildsCharacter::_process(double delta) {
    Dictionary raycast = perform_raycast();
    current_tool->update(raycast);
}

void OuterWildsCharacter::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    float speed = 1.2f;

    Vector3 velocity = get_velocity();

    if (!is_on_floor()) {
        velocity.y -= gravity * (float)delta;
    }

    Input *input = Input::get_singleton();
    if (input->is_action_just_pressed("ui_accept")) { // <- && is_on_floor()
        velocity.y += 4.5;
    }

    if (input->is_action_pressed("run")) {
        speed = 6.0f;
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
    if (Engine::get_singleton()->is_editor_hint()) return;

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

    if (event->is_action_pressed("first_action")) {
        Dictionary result = perform_raycast();

        if (!result.is_empty()) {
            Vector3 hit_point     = result["position"];
            // Vector3 hit_normal    = result["normal"];
            Object *hit_object    = Object::cast_to<Object>(result["collider"]);

            if (hit_object) {
                // Например, вызвать метод на объекте
                // hit_object->call("on_interact");
                // UtilityFunctions::print("Hit: ", hit_object->get_class());
                // UtilityFunctions::print("At:  ", hit_point);

                if(current_tool->can_use_on(result)) {
                    current_tool->use(result);
                }
            }
        }
    }

    if (event->is_action_pressed("second_action")) {
        Dictionary result = perform_raycast();

        if (!result.is_empty()) {
            Vector3 hit_point     = result["position"];
            // Vector3 hit_normal    = result["normal"];
            Object *hit_object    = Object::cast_to<Object>(result["collider"]);

            if (hit_object) {
                // Например, вызвать метод на объекте
                // hit_object->call("on_interact");
                // UtilityFunctions::print("Hit: ", hit_object->get_class());
                // UtilityFunctions::print("At:  ", hit_point);

                if(current_tool->can_use_alt_on(result)) {
                    current_tool->use_alt(result);
                }
            }
        }
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

Dictionary OuterWildsCharacter::perform_raycast(float max_distance) {
    // Получаем начало и направление луча из камеры
    Vector3 ray_origin = camera->get_global_position();
    Vector3 ray_direction = -camera->get_global_transform().basis.get_column(2); // forward
    Vector3 ray_end = ray_origin + ray_direction * max_distance;

    // Параметры запроса
    Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(ray_origin, ray_end);
    query->set_collide_with_bodies(true);
    // query->set_exclude({ get_rid() }); // исключить самого персонажа, если нужно

    // Выполняем запрос
    PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();
    return space_state->intersect_ray(query);
}