#pragma once
#include <godot_cpp/classes/node3d.hpp>

namespace godot {

class Tool : public Node3D {
    GDCLASS(Tool, Node3D)
protected:
    static void _bind_methods() {}
public:
    virtual bool can_use_on    (const Dictionary &raycast_result) const { return false; }
    virtual bool can_use_alt_on(const Dictionary &raycast_result) const { return false; }
    virtual void use    (const Dictionary &raycast_result) {}
    virtual void use_alt(const Dictionary &raycast_result) {}

    virtual void update(const Dictionary &raycast_result) {}
};

} // namespace godot