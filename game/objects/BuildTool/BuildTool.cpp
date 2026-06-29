#include "BuildTool.hpp"

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/shape3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {

void BuildTool::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_build_scene", "scene"), &BuildTool::set_build_scene);
    ClassDB::bind_method(D_METHOD("get_build_scene"),          &BuildTool::get_build_scene);
    ClassDB::bind_method(D_METHOD("update", "raycast_result"), &BuildTool::update);
    ClassDB::bind_method(D_METHOD("clear_preview"),            &BuildTool::clear_preview);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "build_scene",
                              PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_build_scene", "get_build_scene");
}

void BuildTool::use(const Dictionary &raycast_result) {
    if (!preview_valid || build_scene.is_null() || !preview_node) return;
    Node3D* instance = Object::cast_to<Node3D>(build_scene->instantiate());
    if (!instance) return;
    Node3D* parent = Object::cast_to<Node3D>(raycast_result["collider"]);
    if(!parent) return;

    parent->add_child(instance);
    instance->set_owner(parent);
    instance->set_global_transform(preview_node->get_global_transform());
}

void BuildTool::use_alt(const Dictionary &raycast_result) {
    Ref<PackedScene> new_scene = ResourceLoader::get_singleton()->load("res://objects/build/Wall.tscn");
    set_build_scene(new_scene);
}

void BuildTool::update(const Dictionary &raycast_result) {
    if (build_scene.is_null()) { destroy_preview(); return; }
    if (!preview_node) spawn_preview();
    if (!preview_node) return;

    Vector3 pos = raycast_result.has("position")
        ? Vector3(raycast_result["position"])
        : get_global_position() - get_global_transform().basis.get_column(2) * place_distance;
    preview_node->set_global_position(pos);

    bool new_valid = !check_overlap() && preview_node->get_position().length() <= place_distance;
    if (new_valid != preview_valid) {
        preview_valid = new_valid;
        Ref<StandardMaterial3D> &mat = preview_valid ? mat_valid : mat_invalid;
        TypedArray<Node> meshes = preview_node->find_children("*", "MeshInstance3D");
        for (int i = 0; i < (int)meshes.size(); ++i)
            if (MeshInstance3D *mi = Object::cast_to<MeshInstance3D>(meshes[i]))
                mi->set_material_overlay(mat);
    }
}

bool BuildTool::check_overlap() const {
    CollisionShape3D *col = nullptr;
    for (int i = 0; i < preview_node->get_child_count() && !col; ++i)
        col = Object::cast_to<CollisionShape3D>(preview_node->get_child(i));

    Ref<Shape3D> shape = col ? col->get_shape() : Ref<Shape3D>();
    if (shape.is_null()) return false;

    Ref<PhysicsShapeQueryParameters3D> params;
    params.instantiate();
    params->set_shape(shape);
    params->set_transform(preview_node->get_global_transform());
    params->set_collision_mask(0xFFFFFFFF);
    return get_world_3d()->get_direct_space_state()->intersect_shape(params, 1).size() > 0;
}

void BuildTool::spawn_preview() {
    preview_node = Object::cast_to<Node3D>(build_scene->instantiate());
    if (!preview_node) return;
    for (int i = 0; i < (int)preview_node->find_children("*", "CollisionObject3D").size(); ++i)
        Object::cast_to<Node>(preview_node->find_children("*", "CollisionObject3D")[i])
            ->set_process_mode(Node::PROCESS_MODE_DISABLED);
    add_child(preview_node);

    // Создаём материалы один раз
    mat_valid.instantiate();
    mat_valid->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
    mat_valid->set_albedo(Color(0.2f, 1.0f, 0.2f, 0.45f));

    mat_invalid.instantiate();
    mat_invalid->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
    mat_invalid->set_albedo(Color(1.0f, 0.2f, 0.2f, 0.45f));
}

void BuildTool::destroy_preview() {
    if (preview_node) { preview_node->queue_free(); preview_node = nullptr; }
    preview_valid = false;
}

void BuildTool::clear_preview() { destroy_preview(); }

void BuildTool::set_build_scene(const Ref<PackedScene> &scene) {
    build_scene = scene;
    destroy_preview();
}

void BuildTool::_notification(int p_what) {
    if (p_what == NOTIFICATION_EXIT_TREE) destroy_preview();
}

} // namespace godot