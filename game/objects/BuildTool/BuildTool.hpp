#pragma once
#include <Tool/Tool.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {

class BuildTool : public Tool {
    GDCLASS(BuildTool, Tool)

    Ref<PackedScene> build_scene;
    Node3D          *preview_node  = nullptr;
    float            place_distance = 7.5f;
    bool             preview_valid  = false;

    Ref<StandardMaterial3D> mat_valid;
    Ref<StandardMaterial3D> mat_invalid;

protected:
    static void _bind_methods();

public:
    BuildTool() {}

    bool can_use_on    (const Dictionary &raycast_result) const override { return true; }
    bool can_use_alt_on(const Dictionary &raycast_result) const override { return true; }
    void use    (const Dictionary &raycast_result) override;
    void use_alt(const Dictionary &raycast_result) override;

    void update(const Dictionary &raycast_result) override;
    void clear_preview();

    void             set_build_scene(const Ref<PackedScene> &scene);
    Ref<PackedScene> get_build_scene() const { return build_scene; }

    void  set_place_distance(float d) { place_distance = d; }
    float get_place_distance() const  { return place_distance; }

    void _notification(int p_what);

private:
    void spawn_preview();
    void destroy_preview();
    bool check_overlap() const;
};

} // namespace godot