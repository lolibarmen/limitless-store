#pragma once
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <Tool/Tool.hpp>

namespace godot {

class PickableTool : public StaticBody3D {
    GDCLASS(PickableTool, StaticBody3D)

private:
    Ref<PackedScene> tool_scene;
    Node *tool_instance = nullptr;

    StringName tool_class;

    void _update_preview();

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    PickableTool();

    void set_tool_scene(const Ref<PackedScene> &scene);
    Ref<PackedScene> get_tool_scene() const;

    void set_tool_class(const StringName &cls);
    StringName get_tool_class() const;

    Tool *create_tool() const;
};

}