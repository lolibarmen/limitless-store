#include "TestMeshInstance3D.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TestMeshInstance3D::_bind_methods() {
    // Можно добавить кастомные методы для вызова из GDScript, если нужно
}

TestMeshInstance3D::TestMeshInstance3D() {
    UtilityFunctions::print(">>> TestMeshInstance3D CONSTRUCTOR called, ID: ", get_instance_id());
}

TestMeshInstance3D::~TestMeshInstance3D() {
    UtilityFunctions::print("!!! TestMeshInstance3D DESTRUCTOR called, ID: ", get_instance_id());
    UtilityFunctions::print("!!! Mesh was set: ", mesh_was_set ? "YES" : "NO");
}

void TestMeshInstance3D::_ready() {
    UtilityFunctions::print(">>> TestMeshInstance3D _ready called, ID: ", get_instance_id());
    UtilityFunctions::print("Parent: ", get_parent() ? get_parent()->get_name() : "NULL");
    UtilityFunctions::print("is_inside_tree: ", is_inside_tree() ? "YES" : "NO");
}

void TestMeshInstance3D::_process(double delta) {
    frame_count++;
    
    // Логируем КАЖДЫЙ кадр первые 10 кадров
    if (frame_count <= 10) {
        UtilityFunctions::print(">>> TestMeshInstance3D _process FRAME ", frame_count, 
                              ", ID: ", get_instance_id(),
                              ", is_inside_tree: ", is_inside_tree() ? "YES" : "NO",
                              ", is_visible_in_tree: ", is_visible_in_tree() ? "YES" : "NO",
                              ", mesh valid: ", get_mesh().is_valid() ? "YES" : "NO");
    }
}

void TestMeshInstance3D::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE:
            UtilityFunctions::print(">>> TestMeshInstance3D ENTER_TREE, ID: ", get_instance_id());
            break;
            
        case NOTIFICATION_EXIT_TREE:
            UtilityFunctions::print("!!! TestMeshInstance3D EXIT_TREE, ID: ", get_instance_id());
            UtilityFunctions::print("!!! Reason: Node removed from tree!");
            break;
            
        case NOTIFICATION_PREDELETE:
            UtilityFunctions::print("!!! TestMeshInstance3D PREDELETE (destroying), ID: ", get_instance_id());
            break;
            
        case NOTIFICATION_VISIBILITY_CHANGED:
            UtilityFunctions::print("TestMeshInstance3D VISIBILITY_CHANGED, ID: ", get_instance_id(),
                                  ", visible: ", is_visible() ? "YES" : "NO");
            break;
    }
}

void TestMeshInstance3D::set_mesh(const Ref<Mesh> &p_mesh) {
    UtilityFunctions::print(">>> TestMeshInstance3D set_mesh called, ID: ", get_instance_id());
    UtilityFunctions::print("    Mesh valid: ", p_mesh.is_valid() ? "YES" : "NO");
    
    if (p_mesh.is_valid()) {
        UtilityFunctions::print("    Surface count: ", p_mesh->get_surface_count());
        UtilityFunctions::print("    AABB: ", p_mesh->get_aabb());
        mesh_was_set = true;
    }
    
    // Вызываем базовый метод
    MeshInstance3D::set_mesh(p_mesh);
    
    UtilityFunctions::print("    After base set_mesh, get_mesh() valid: ", 
                           MeshInstance3D::get_mesh().is_valid() ? "YES" : "NO");
}

Ref<Mesh> TestMeshInstance3D::get_mesh() const {
    Ref<Mesh> mesh = MeshInstance3D::get_mesh();
    return mesh;
}

void TestMeshInstance3D::_enter_tree() {
    UtilityFunctions::print(">>> TestMeshInstance3D _enter_tree called, ID: ", get_instance_id());
}

void TestMeshInstance3D::_exit_tree() {
    UtilityFunctions::print("!!! TestMeshInstance3D _exit_tree called, ID: ", get_instance_id());
}