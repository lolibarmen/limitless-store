#include "BiomeGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void BiomeGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_seed"), &BiomeGenerator::get_seed);
}

void BiomeGenerator::init(int p_seed) {
    seed = p_seed;
    setup_noise();
}

void BiomeGenerator::setup_noise() {
    noise_humidity.instantiate();
    noise_humidity->set_seed(seed);
    noise_humidity->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_humidity->set_frequency(humidity_scale);

    noise_temperature.instantiate();
    noise_temperature->set_seed(seed + 1);
    noise_temperature->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_temperature->set_frequency(temperature_scale);

    noise_height.instantiate();
    noise_height->set_seed(seed + 2);
    noise_height->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_height->set_frequency(height_scale);
}

BiomeType BiomeGenerator::classify(float humidity, float temperature, float height) const {
    if (height < -0.3f)                          return BiomeType::OCEAN;
    if (height >  0.6f)                          return BiomeType::MOUNTAIN;
    if (temperature < -0.3f)                     return BiomeType::TUNDRA;
    if (humidity < -0.2f && temperature > 0.2f)  return BiomeType::DESERT;
    if (humidity >  0.1f && temperature > -0.1f) return BiomeType::FOREST;
    return BiomeType::PLAINS;
}

BiomeData BiomeGenerator::get_biome(const Vector3i& world_pos) const {
    ERR_FAIL_COND_V_MSG(
        !noise_humidity.is_valid() || !noise_temperature.is_valid() || !noise_height.is_valid(),
        BiomeData{},
        "BiomeGenerator: не инициализирован, вызови init()"
    );

    float h = noise_humidity->get_noise_2d(world_pos.x, world_pos.z);
    float t = noise_temperature->get_noise_2d(world_pos.x, world_pos.z);
    float e = noise_height->get_noise_2d(world_pos.x, world_pos.z);

    BiomeData data;
    data.humidity    = h;
    data.temperature = t;
    data.height      = e;
    data.type        = classify(h, t, e);

    return data;
}