#include "ToolManager.hpp"
#include <Tool/Tool.hpp>
#include <PickableTool/PickableTool.hpp>
#include <godot_cpp/classes/engine.hpp>

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

void ToolManager::pick_up(const Dictionary& raycast_result) {
    print_line("Start pickaping");
    Object* collider = Object::cast_to<Object>(raycast_result["collider"]);
    if (!collider) return;

    print_line("Shoot detecting");
    print_line(collider);

    PickableTool* item = Object::cast_to<PickableTool>(collider);
    if (!item) return;

    print_line("Object is PickableTool");

    Tool* new_tool = item->create_tool();
    if (!new_tool) {
        print_error("PickableItem: create_tool return nullptr");
        return;
    }

    // Удаляем предмет из мира
    item->queue_free();

    // Заменяем текущий инструмент
    set_tool(new_tool);
}

Tool* ToolManager::get_current_tool() const {
    return current_tool;
}

bool ToolManager::has_tool() const {
    return current_tool != nullptr;
}