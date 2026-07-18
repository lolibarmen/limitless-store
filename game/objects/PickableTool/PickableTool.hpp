#pragma once
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <Tool/Tool.hpp>

namespace godot {

class PickableTool : public StaticBody3D {
    GDCLASS(PickableTool, StaticBody3D)

private:
    StringName tool_class;

protected:
    static void _bind_methods();

public:
    PickableTool();

    void set_tool_class(const StringName &cls);
    StringName get_tool_class() const;

    Tool *create_tool();
};

}