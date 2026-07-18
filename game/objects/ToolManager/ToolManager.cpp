#include "ToolManager.hpp"
#include <Tool/Tool.hpp>
#include <PickableTool/PickableTool.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

ToolManager::ToolManager() {}

void ToolManager::_bind_methods() {}

void ToolManager::set_tool(Tool* tool) {
    if (current_tool) {
        remove_child(current_tool);
        current_tool->queue_free();
        current_tool = nullptr;
    }
    if (tool) {
        add_child(tool);
        current_tool = tool;
    }
}

void ToolManager::update_tool(const Dictionary& raycast_result) {
    if (current_tool) {
        current_tool->update(raycast_result);
    }
}

void ToolManager::drop_current_tool(const Transform3D &drop_transform) {
    if (!current_tool) {
        return;
    }

    PickableTool* pickable = Object::cast_to<PickableTool>(
        current_tool->get_pickable_scene()->instantiate()
    );
    pickable->set_tool_class(current_tool->get_class());

    Node* world = get_tree()->get_current_scene();
    world->add_child(pickable);
    pickable->set_global_transform(drop_transform);

    remove_child(current_tool);
    current_tool->queue_free();
    current_tool = nullptr;
}


void ToolManager::pick_up(const Dictionary& raycast_result) {
    Object* collider = Object::cast_to<Object>(raycast_result["collider"]);
    if (!collider) return;

    PickableTool* item = Object::cast_to<PickableTool>(collider);
    if (!item) return;

    Tool* new_tool = item->create_tool();
    if (!new_tool) {
        print_error("PickableItem: create_tool return nullptr");
        return;
    }

    Transform3D drop_transform = item->get_global_transform();
    item->queue_free();

    if (has_tool()) {
        drop_current_tool(drop_transform); // старый инструмент падает туда, где лежал новый
    }

    set_tool(new_tool);
}

Tool* ToolManager::get_current_tool() const {
    return current_tool;
}

bool ToolManager::has_tool() const {
    return current_tool != nullptr;
}