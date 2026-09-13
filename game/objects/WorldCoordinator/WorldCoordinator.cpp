#include "WorldCoordinator.hpp"
#include <SurfaceGenerator/ChunkMaterialManager.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <StoneSphere/StoneSphere.hpp>

using namespace godot;

void WorldCoordinator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_seed"), &WorldCoordinator::get_seed);
    ClassDB::bind_method(D_METHOD("set_seed", "v"), &WorldCoordinator::set_seed);
    ClassDB::add_property("WorldCoordinator",
        PropertyInfo(Variant::INT, "seed"),
        "set_seed", "get_seed");
        
    // Метод все еще полезен для привязки, если вы хотите вызывать его из GDScript
    ClassDB::bind_method(D_METHOD("_register_initial_shapes"), &WorldCoordinator::_register_initial_shapes);
}

void WorldCoordinator::_ready() {
    ChunkMaterialManager::get_singleton().initialize();

    WorldMesh* wm = WorldMesh::get_singleton();
    add_child(wm);

    // Вместо call_deferred включаем ежедневный вызов _process для подсчета кадров
    set_process(true);
}

void WorldCoordinator::_process(double delta) {
    frame_counter++;
    
    if (frame_counter >= 10) {
        set_process(false);
        _register_initial_shapes();
    }
}

void WorldCoordinator::_register_initial_shapes() {
    Ref<StoneSphere> stone_sphere;
    stone_sphere.instantiate();

    stone_sphere->set_radius(10.0f);
    stone_sphere->set_center(Vector3(0, 0, 0));

    SemanticWorld* sw = SemanticWorld::get_singleton();
    sw->register_shape(stone_sphere);
}