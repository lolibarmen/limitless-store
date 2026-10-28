#include "WorldCoordinator.hpp"
#include <SurfaceGenerator/ChunkMaterialManager.hpp>
#include <cstdint>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <SpatialStreaming/SpatialStreaming.hpp>

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

    SpatialStreaming* ss = SpatialStreaming::get_singleton();
    add_child(ss);

    WorldMesh* wm = WorldMesh::get_singleton();
    add_child(wm);
    
    set_process(true);
}

void WorldCoordinator::_process(double delta) {
    frame_counter++;
    
    if (frame_counter >= 10) {
        set_process(false);
        _register_initial_shapes();
    }
}

// #include <TreeBranch/TreeBranch.hpp>
// #include <TreeTrunk/TreeTrunk.hpp>
#include <TestCube/TestCube.hpp>
#include <MaterialTerrainShape/MaterialTerrainShape.hpp>
void WorldCoordinator::_register_initial_shapes() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return;

    // Ref<TestCube> cube;
    // cube.instantiate();
    // sw->register_shape(cube);
    
    Ref<MaterialTerrainShape> terr;
    terr.instantiate();    
    sw->register_shape(terr);
}