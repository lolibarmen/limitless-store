#pragma once

#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

using namespace godot;

class TestTreeNode : public StaticBody3D {
    GDCLASS(TestTreeNode, StaticBody3D)

private:
    MeshInstance3D *mesh_instance;
    CollisionShape3D *collision_shape;

protected:
    static void _bind_methods();

public:
    TestTreeNode();
    ~TestTreeNode() override = default;

    void _ready() override;
};