#include "PlayerInteraction.hpp"
#include <ToolManager/ToolManager.hpp>
#include <Tool/Tool.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

PlayerInteraction::PlayerInteraction() {}

void PlayerInteraction::_bind_methods() {}

Dictionary PlayerInteraction::perform_raycast(Camera3D* camera, Node* parent, float max_distance) {
    Vector3 ray_origin = camera->get_global_position();
    Vector3 ray_direction = -camera->get_global_transform().basis.get_column(2); // forward
    Vector3 ray_end = ray_origin + ray_direction * max_distance;

    Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(ray_origin, ray_end);
    query->set_collide_with_bodies(true);
    // query->set_exclude({ get_rid() }); // исключить самого персонажа, если нужно

    PhysicsDirectSpaceState3D *space_state = camera->get_world_3d()->get_direct_space_state();
    return space_state->intersect_ray(query);
}

void PlayerInteraction::process_input(Camera3D* camera, ToolManager* tool_mgr, const Ref<InputEvent>& event) {
    if (Engine::get_singleton()->is_editor_hint()) return;
    if (!camera || !tool_mgr) return;

    Tool* tool = tool_mgr->get_current_tool();
    if (!tool) return;

    if (event->is_action_pressed("first_action")) {
        Dictionary result = perform_raycast(camera, camera->get_parent());
        if (!result.is_empty()) {
            if (tool->can_use_on(result)) {
                tool->use(result);
            }
        }
    }

    if (event->is_action_pressed("second_action")) {
        Dictionary result = perform_raycast(camera, camera->get_parent());
        if (!result.is_empty()) {
            if (tool->can_use_alt_on(result)) {
                tool->use_alt(result);
            }
        }
    }

    if (event->is_action_pressed("pick_up")) {
        Dictionary result = perform_raycast(camera, camera->get_parent());
        if (!result.is_empty()) {
            tool_mgr->pick_up(result);
        }
    }
}