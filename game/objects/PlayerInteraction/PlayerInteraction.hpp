#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {

class ToolManager;

class PlayerInteraction : public RefCounted {
    GDCLASS(PlayerInteraction, RefCounted)

protected:
    static void _bind_methods();

public:
    PlayerInteraction();
    Dictionary perform_raycast(Camera3D* camera, Node* parent, float max_distance = 1000.0f);
    void process_input(Camera3D* camera, ToolManager* tool_mgr, const Ref<InputEvent>& event);
};

}