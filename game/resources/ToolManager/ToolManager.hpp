#pragma once
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {

class Tool;
class PickableTool;

class ToolManager : public Node {
    GDCLASS(ToolManager, Node)

private:
    Tool* current_tool = nullptr;

protected:
    static void _bind_methods();

public:
    ToolManager();
    void set_tool(Tool* tool);
    void update_tool(const Dictionary& raycast_result);
    void pick_up(const Dictionary& raycast_result);

    Tool* get_current_tool() const;
    bool has_tool() const;
};

}