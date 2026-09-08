#include "BuildTool.hpp"

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/shape3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

using namespace godot;

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
    // if (!preview_valid || build_scene.is_null() || !preview_node) return;

    // Node3D* instance = Object::cast_to<Node3D>(build_scene->instantiate());
    // if (!instance) return;

    // if (target_socket) {
    //     Node3D* parent = Object::cast_to<Node3D>(target_socket->get_parent());
    //     if (!parent) { instance->queue_free(); return; }

    //     // Позиция берётся напрямую у preview_node — гарантированно совпадает
    //     // с тем, что видел игрок, без повторного пересчёта.
    //     Transform3D new_xform = preview_node->get_global_transform();

    //     parent->add_child(instance);
    //     instance->set_owner(parent);
    //     instance->set_global_transform(new_xform);

    //     // entry_socket ищем отдельно только для удаления — на transform не влияет
    //     BuildSocket *entry_socket = find_entry_socket(instance, target_socket);

    //     target_socket->queue_free();
    //     if (entry_socket) entry_socket->queue_free();
    //     target_socket = nullptr;
    //     return;
    // }

    // Node3D* parent = Object::cast_to<Node3D>(raycast_result["collider"]);
    // if (!parent) return;

    // parent->add_child(instance);
    // instance->set_owner(parent);
    // instance->set_global_transform(preview_node->get_global_transform());
}

void BuildTool::use_alt(const Dictionary &raycast_result) {
    // Ref<PackedScene> new_scene = ResourceLoader::get_singleton()->load("res://objects/build/Wall.tscn");
    // set_build_scene(new_scene);
}

// Ищет в instance сокет, которым он должен состыковаться с target_socket.
BuildSocket* BuildTool::find_entry_socket(Node3D *p_instance, BuildSocket *p_target_socket) const {
    // if (!p_instance || !p_target_socket) return nullptr;

    // TypedArray<Node> candidates = p_instance->find_children("*", "BuildSocket");
    // String want_id = p_target_socket->get_connects_to_id();

    // for (int i = 0; i < candidates.size(); ++i) {
    //     BuildSocket *cand = Object::cast_to<BuildSocket>(candidates[i]);
    //     if (cand && !want_id.is_empty() && cand->get_socket_id() == want_id)
    //         return cand;
    // }
    // // Фолбэк: id не задан или совпадений нет — берём первый попавшийся сокет
    // if (candidates.size() > 0)
    //     return Object::cast_to<BuildSocket>(candidates[0]);

    // return nullptr;
}

// Вычисляет transform для p_instance так, чтобы p_entry_socket совпал с target_socket.
Transform3D BuildTool::compute_socket_attach_transform(Node3D *p_instance, BuildSocket *p_target_socket, BuildSocket *p_entry_socket) const {
    // if (!p_instance || !p_target_socket)
    //     return p_instance ? p_instance->get_global_transform() : Transform3D();

    // if (!p_entry_socket)
    //     return p_target_socket->get_global_transform();

    // Transform3D socket_xform = p_target_socket->get_global_transform();
    // Transform3D entry_local = p_instance->get_global_transform().affine_inverse()
    //                          * p_entry_socket->get_global_transform();

    // return socket_xform * entry_local.affine_inverse();
}

void BuildTool::update(const Dictionary &raycast_result) {
    // if (build_scene.is_null()) { destroy_preview(); return; }
    // if (!preview_node) spawn_preview();
    // if (!preview_node) return;
    
    // // Проверяем, не смотрит ли камера на сокет
    // target_socket = nullptr;
    // if (raycast_result.has("collider")) {
    //     target_socket = Object::cast_to<BuildSocket>(
    //         Object::cast_to<Object>(raycast_result["collider"]));
    // }

    // if (target_socket) {
    //     BuildSocket *entry_socket = find_entry_socket(preview_node, target_socket);
    //     preview_node->set_global_transform(
    //         compute_socket_attach_transform(preview_node, target_socket, entry_socket));
    // } else {
    //     Vector3 pos = raycast_result.has("position")
    //         ? Vector3(raycast_result["position"])
    //         : get_global_position() - get_global_transform().basis.get_column(2) * place_distance;
    //     preview_node->set_global_position(pos);
    //     preview_node->set_global_rotation(Vector3(0.0, 0.0, 0.0));
    // }

    // // Если попали в сокет — считаем позицию валидной без проверки overlap
    // // (сокет и так помечает точку установки, а его собственный коллайдер
    // // иначе постоянно засчитывался бы как пересечение)
    // bool new_valid = target_socket
    //     ? preview_node->get_position().length() <= place_distance
    //     : (!check_overlap() && preview_node->get_position().length() <= place_distance);

    // if (new_valid != preview_valid) {
    //     preview_valid = new_valid;
    //     Ref<StandardMaterial3D> &mat = preview_valid ? mat_valid : mat_invalid;
    //     TypedArray<Node> meshes = preview_node->find_children("*", "MeshInstance3D");
    //     for (int i = 0; i < (int)meshes.size(); ++i)
    //         if (MeshInstance3D *mi = Object::cast_to<MeshInstance3D>(meshes[i]))
    //             mi->set_material_overlay(mat);
    // }
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
    target_socket = nullptr;
}

void BuildTool::clear_preview() { destroy_preview(); }

void BuildTool::set_build_scene(const Ref<PackedScene> &scene) {
    build_scene = scene;
    destroy_preview();
}

void BuildTool::_notification(int p_what) {
    if (p_what == NOTIFICATION_EXIT_TREE) destroy_preview();
}