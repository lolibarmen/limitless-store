#include "ChunkCollider.hpp"
#include <ChunkNode/ChunkNode.hpp>

#include <godot_cpp/classes/concave_polygon_shape3d.hpp>

using namespace godot;

ChunkCollider::ChunkCollider() {}

ChunkCollider::~ChunkCollider() {}

void ChunkCollider::_ready() {
    // Создаём CollisionShape3D как дочерний узел
    collision_shape = memnew(CollisionShape3D);
    add_child(collision_shape);
}

void ChunkCollider::set_mesh(const Ref<ArrayMesh> mesh) {
    if (mesh.is_valid()) {
        Ref<ConcavePolygonShape3D> shape;
        shape.instantiate();
        shape->set_faces(mesh->get_faces());
        collision_shape->set_shape(shape);
    }
}