#include "WorldCoordinator.hpp"
#include <BlockSource/BlockSource.hpp>
#include <ChunkMeshNode/ChunkMaterialManager.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void WorldCoordinator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_seed"),       &WorldCoordinator::get_seed);
    ClassDB::bind_method(D_METHOD("set_seed", "v"),  &WorldCoordinator::set_seed);
    ClassDB::add_property("WorldCoordinator",
        PropertyInfo(Variant::INT, "seed"),
        "set_seed", "get_seed");
}

void WorldCoordinator::_ready() {
    ChunkMaterialManager::get_singleton().initialize();

    // --- передаём менеджеру ---
    chunk_manager = memnew(NeochunkManager);
    add_child(chunk_manager);
}