#include "BiomeGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void BiomeGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_seed"),      &BiomeGenerator::get_seed);
    ClassDB::bind_method(D_METHOD("set_seed", "v"), &BiomeGenerator::set_seed);
    ClassDB::add_property("BiomeGenerator",
        PropertyInfo(Variant::INT, "seed"),
        "set_seed", "get_seed");
}

void BiomeGenerator::setup_noise() {
    // Заглушка — шумы не нужны
}

BiomeType BiomeGenerator::classify(float humidity, float temperature, float height) const {
    return BiomeType::PLAINS;
}

void BiomeGenerator::init(int p_seed) {
    seed = p_seed;
}

BiomeData BiomeGenerator::get_biome(const Vector3i& world_pos) const {
    BiomeData biome;
    biome.type        = BiomeType::PLAINS;
    biome.humidity    = 0.5f;
    biome.temperature = 0.5f;
    biome.height      = 0.5f;
    return biome;
}