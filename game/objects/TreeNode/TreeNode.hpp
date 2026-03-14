#pragma once

#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

namespace godot {

class TreeNode : public StaticBody3D {
    GDCLASS(TreeNode, StaticBody3D)

private:
    MeshInstance3D *mesh_instance;
    CollisionShape3D *collision_shape;

protected:
    static void _bind_methods();

public:
    TreeNode();
    ~TreeNode() override = default;

    void _ready() override;
};

} // namespace godot