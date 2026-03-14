#include "TreeNode.hpp"

#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>

#include <cstdio>

using namespace godot;

void TreeNode::_bind_methods() {
}

TreeNode::TreeNode() {
    mesh_instance = memnew(MeshInstance3D);
    collision_shape = memnew(CollisionShape3D);

    add_child(mesh_instance);
    mesh_instance->set_owner(this);
    
    add_child(collision_shape);
    collision_shape->set_owner(this);
}

void TreeNode::_ready() {
    // Создаём простой куб как дерево
    Ref<BoxMesh> box_mesh = memnew(BoxMesh);
    box_mesh->set_size(Vector3(1, 3, 1));
    
    // Создаём зелёный материал
    Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
    material->set_albedo(Color(0.2, 0.8, 0.2));  // Просто зелёный цвет
    box_mesh->set_material(material);
    
    mesh_instance->set_mesh(box_mesh);
    mesh_instance->set_position(Vector3(0, 1.5, 0));
    
    // Коллизия
    Ref<BoxShape3D> box_shape = memnew(BoxShape3D);
    box_shape->set_size(Vector3(1, 3, 1));
    collision_shape->set_shape(box_shape);
    collision_shape->set_position(Vector3(0, 1.5, 0));
}