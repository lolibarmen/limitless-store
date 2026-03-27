#include "WorldCoordinator.hpp"
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
    // --- уровень 1: биом ---
    Ref<BiomeGenerator> biome_gen;
    biome_gen.instantiate();
    biome_gen->init(seed);

    Ref<BiomeSource> biome_source;
    biome_source.instantiate();
    biome_source->set_generator(biome_gen);

    // --- уровень 2: блоки ---
    Ref<BlockGenerator> block_gen;
    block_gen.instantiate();
    block_gen->init(biome_source, seed);

    Ref<BlockLODSource> block_lod3_source;
    block_lod3_source.instantiate();
    block_lod3_source->init(block_gen, Ref<BlockGenerator>());

    Ref<BlockLODSource> block_lod2_source;
    block_lod2_source.instantiate();
    block_lod2_source->init(block_gen, block_lod3_source);

    Ref<BlockLODSource> block_lod1_source;
    block_lod1_source.instantiate();
    block_lod1_source->init(block_gen, block_lod2_source);

    // --- передаём менеджеру ---
    chunk_manager = memnew(ChunkManager);
    chunk_manager->init(block_lod1_source, block_lod2_source, block_lod3_source);
    add_child(chunk_manager);
}