#include "BlockSource.hpp"

using namespace godot;

void BlockSource::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_generator"),         &BlockSource::get_generator);
    ClassDB::bind_method(D_METHOD("set_generator", "gen"),  &BlockSource::set_generator);
    ClassDB::add_property("BlockSource",
        PropertyInfo(Variant::OBJECT, "generator",
            PROPERTY_HINT_RESOURCE_TYPE, "BlockGenerator"),
        "set_generator", "get_generator");
}

void BlockSource::init(Ref<BiomeSource> biome_source, int seed) {
    ERR_FAIL_COND_MSG(!biome_source.is_valid(),
        "BlockSource::init: biome_source невалиден");

    Ref<BlockGenerator> gen;
    gen.instantiate();
    gen->init(biome_source, seed);
    generator = gen;
}

BlockData BlockSource::get_block(const Vector3i& world_pos) const {
    auto it = edits.find(world_pos);
    if (it != edits.end())
        return it->second;

    ERR_FAIL_COND_V_MSG(!generator.is_valid(), BlockData{},
        "BlockSource: generator не задан, вызови init()");

    return generator->get_block(world_pos);
}

void BlockSource::set_block(const Vector3i& world_pos, BlockData data) {
    // Опционально: клампим density на случай кривых данных
    data.density = Math::clamp(data.density, 0.0f, 1.0f);
    edits[world_pos] = data;
}

void BlockSource::reset_block(const Vector3i& world_pos) {
    edits.erase(world_pos);
}

bool BlockSource::has_edit(const Vector3i& world_pos) const {
    return edits.count(world_pos) > 0;
}