#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

namespace godot {

class Tool : public Node3D {
    GDCLASS(Tool, Node3D)
private:
    Ref<PackedScene> pickable_scene;

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_pickable_scene", "scene"), &Tool::set_pickable_scene);
        ClassDB::bind_method(D_METHOD("get_pickable_scene"), &Tool::get_pickable_scene);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "pickable_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                     "set_pickable_scene", "get_pickable_scene");
    }

public:
    void set_pickable_scene(const Ref<PackedScene> &scene) { pickable_scene = scene; }
    Ref<PackedScene> get_pickable_scene() const { return pickable_scene; }

    virtual bool can_use_on    (const Dictionary &raycast_result) const { return false; }
    virtual bool can_use_alt_on(const Dictionary &raycast_result) const { return false; }
    virtual void use    (const Dictionary &raycast_result) {}
    virtual void use_alt(const Dictionary &raycast_result) {}

    virtual void update(const Dictionary &raycast_result) {}
};

} // namespace godot