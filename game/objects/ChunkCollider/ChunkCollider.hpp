#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

#include <godot_cpp/classes/array_mesh.hpp>

namespace godot {

class ChunkNode;

class ChunkCollider : public StaticBody3D {
    GDCLASS(ChunkCollider, StaticBody3D)

private:

    CollisionShape3D *collision_shape = nullptr;

protected:
    static void _bind_methods() {};

public:

    ChunkCollider();
    ~ChunkCollider();

    void _ready() override;

    void set_mesh(const Ref<ArrayMesh> mesh);
};

} // namespace godot