#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot {

class PlayerCamera : public RefCounted {
    GDCLASS(PlayerCamera, RefCounted)

private:
    float sensitivity = 0.002f;
    float pitch_min = -Math_PI / 2.0 + 0.1;
    float pitch_max = Math_PI / 2.0 - 0.1;

protected:
    static void _bind_methods();

public:
    PlayerCamera();
    void setup_camera(Node3D* parent);
    void process_input(Node3D* parent, const Ref<InputEvent>& event);
    void toggle_mouse_capture();
    Camera3D* get_camera() const;
    Node3D* get_pivot() const;

private:
    Camera3D* camera = nullptr;
    Node3D* camera_pivot = nullptr;
};

}