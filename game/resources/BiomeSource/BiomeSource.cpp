#include "BiomeSource.hpp"

using namespace godot;

void BiomeSource::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_generator"),          &BiomeSource::get_generator);
    ClassDB::bind_method(D_METHOD("set_generator", "gen"),   &BiomeSource::set_generator);
    ClassDB::add_property("BiomeSource",
        PropertyInfo(Variant::OBJECT, "generator",
            PROPERTY_HINT_RESOURCE_TYPE, "BiomeGenerator"),
        "set_generator", "get_generator");
}

BiomeData BiomeSource::get_biome(const Vector3i& world_pos) const {
    auto it = edits.find(world_pos);
    if (it != edits.end())
        return it->second;

    ERR_FAIL_COND_V_MSG(!generator.is_valid(), BiomeData{},
        "BiomeSource: generator не задан");

    return generator->get_biome(world_pos);
}

void BiomeSource::set_biome(const Vector3i& world_pos, const BiomeData& data) {
    edits[world_pos] = data;
}

void BiomeSource::reset_biome(const Vector3i& world_pos) {
    edits.erase(world_pos);
}

bool BiomeSource::has_edit(const Vector3i& world_pos) const {
    return edits.count(world_pos) > 0;
}