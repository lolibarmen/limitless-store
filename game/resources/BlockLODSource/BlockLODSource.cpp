#include "BlockLODSource.hpp"
#include <stdio.h>

using namespace godot;

void BlockLODSource::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_generator"),         &BlockLODSource::get_generator);
    ClassDB::bind_method(D_METHOD("set_generator", "gen"),  &BlockLODSource::set_generator);
    ClassDB::add_property("BlockLODSource",
        PropertyInfo(Variant::OBJECT, "generator",
            PROPERTY_HINT_RESOURCE_TYPE, "BlockGenerator"),
        "set_generator", "get_generator");
}

void BlockLODSource::init(Ref<BlockGenerator> p_generator, Ref<BlockLODSource> p_block_lod_source) {
    if(!p_generator.is_valid()) {
        print_error("void BlockLODSource::init() p_generator not valid");
    }

    if(!p_block_lod_source.is_valid()) {
        print_error("void BlockLODSource::init() p_block_lod_source not valid");
    }

    generator = p_generator;
    block_lod_source = p_block_lod_source;
}

BlockData BlockLODSource::get_block(const Vector3i& world_pos) const {
    auto it = edits.find(world_pos);
    if (it != edits.end())
        return it->second;

    return generator->get_block(world_pos);
}

void BlockLODSource::set_block(const Vector3i& world_pos, BlockData data) {
    edits[world_pos] = data;
}

void BlockLODSource::reset_block(const Vector3i& world_pos) {
    edits.erase(world_pos);
}

bool BlockLODSource::has_edit(const Vector3i& world_pos) const {
    return edits.count(world_pos) > 0;
}