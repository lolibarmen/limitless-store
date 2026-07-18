#include "PickableTool.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

PickableTool::PickableTool() {}

void PickableTool::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_tool_class", "cls"), &PickableTool::set_tool_class);
    ClassDB::bind_method(D_METHOD("get_tool_class"), &PickableTool::get_tool_class);

    ClassDB::bind_method(D_METHOD("create_tool"), &PickableTool::create_tool);

    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "tool_class"),
                 "set_tool_class", "get_tool_class");
}

void PickableTool::set_tool_class(const StringName &cls) {
    tool_class = cls;
}

StringName PickableTool::get_tool_class() const {
    return tool_class;
}

Tool *PickableTool::create_tool() {
    if (tool_class == StringName()) {
        return nullptr;
    }

    Object *obj = ClassDB::instantiate(tool_class);
    Tool *tool = Object::cast_to<Tool>(obj);

    if (tool == nullptr) {
        print_error("Can't lift the object with name: ", tool_class);
        if (obj != nullptr) {
            memdelete(obj);
        }
        return nullptr;
    }

    tool->set_transform(Transform3D());

    Ref<PackedScene> pickable_scene;
    pickable_scene.instantiate();
    pickable_scene->pack(this);
    tool->set_pickable_scene(pickable_scene);
    print_line(tool);
    return tool;
}