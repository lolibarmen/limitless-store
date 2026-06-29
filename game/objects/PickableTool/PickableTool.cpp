#include "PickableTool.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

PickableTool::PickableTool() {}

void PickableTool::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_tool_scene", "scene"), &PickableTool::set_tool_scene);
    ClassDB::bind_method(D_METHOD("get_tool_scene"), &PickableTool::get_tool_scene);

        ClassDB::bind_method(D_METHOD("set_tool_class", "cls"), &PickableTool::set_tool_class);
    ClassDB::bind_method(D_METHOD("get_tool_class"), &PickableTool::get_tool_class);

    ClassDB::bind_method(D_METHOD("create_tool"), &PickableTool::create_tool);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tool_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_tool_scene", "get_tool_scene");
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "tool_class"),
                 "set_tool_class", "get_tool_class");
}

void PickableTool::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        _update_preview();
    }
}

void PickableTool::set_tool_scene(const Ref<PackedScene> &scene) {
    if (tool_scene == scene) {
        return;
    }
    tool_scene = scene;
    if (is_inside_tree()) {
        _update_preview();
    }
}

void PickableTool::_update_preview() {
    TypedArray<Node> old_children = get_children();
    for (int i = 0; i < old_children.size(); i++) {
        Node* child = Object::cast_to<Node>(old_children[i]);
        if (child) {
            remove_child(child);
            child->queue_free();
        }
    }

    if (tool_scene.is_null()) return;

    Node* instance = tool_scene->instantiate();
    if (instance == nullptr) return;

    add_child(instance);

    TypedArray<Node> children = instance->get_children();
    for (int i = 0; i < children.size(); i++) {
        Node* child = Object::cast_to<Node>(children[i]);
        if (child) {
            child->set_owner(nullptr);
            child->reparent(this);
        }
    }

    remove_child(instance);
    instance->queue_free();

    if (Engine::get_singleton()->is_editor_hint()) {
        Node* edited_root = get_tree()->get_edited_scene_root();
        if (edited_root != nullptr) {
            for (int i = 0; i < get_child_count(); i++) {
                get_child(i)->set_owner(edited_root);
            }
        }
    }
}

Ref<PackedScene> PickableTool::get_tool_scene() const {
    return tool_scene;
}

void PickableTool::set_tool_class(const StringName &cls) {
    tool_class = cls;
}

StringName PickableTool::get_tool_class() const {
    return tool_class;
}

Tool *PickableTool::create_tool() const {
    if (tool_class == StringName()) {
        return nullptr;
    }

    Object *obj = ClassDB::instantiate(tool_class);
    Tool *tool = Object::cast_to<Tool>(obj);

    if (tool == nullptr) {
        print_error("Can't lift the object with name", tool_class);
        if (obj != nullptr) {
            memdelete(obj);
        }
        return nullptr;
    }

    tool->set_transform(Transform3D());
    return tool;
}