#include "BiomeGenerator.hpp"

using namespace godot;

int compute_lod(float distance) {
    if (distance < 64)  return 1;
    if (distance < 128) return 2;
    if (distance < 256) return 4;
    if (distance < 512) return 8;
    return 16;
}

std::vector<Slot> BiomeGenerator::query(const Vector3& camera_pos, float radius) const {
    std::vector<Slot> result;

    int range       = (int)radius;
    int voxel_count = 8;

    for (int x = -range; x < range; x += voxel_count)
    for (int y = -range; y < range; y += voxel_count)
    for (int z = -range; z < range; z += voxel_count)
    {
        Vector3  world_pos(x, y, z);
        float    distance = camera_pos.distance_to(world_pos);
        int      lod      = compute_lod(distance);
        Vector3i origin(x, y, z);

        Dictionary extra;
        extra["origin"] = origin;
        extra["lod"]    = lod;

        Slot slot;
        slot.position = world_pos;
        slot.seed     = seed ^ (x * 73856093 ^ y * 19349663 ^ z * 83492791);
        slot.radius   = voxel_count * lod;
        slot.extra    = extra;

        result.push_back(slot);
    }

    return result;
}

void BiomeGenerator::_bind_methods() {}

void BiomeGenerator::initialize(int p_seed, float p_radius) {
    seed          = p_seed;
    planet_radius = p_radius;

    // --- Terrain noise ---
    terrain_noise = Ref<FastNoiseLite>(memnew(FastNoiseLite));
    terrain_noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    terrain_noise->set_seed(seed);
    terrain_noise->set_frequency(0.8f);       // Крупные формы рельефа
    terrain_noise->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    terrain_noise->set_fractal_octaves(5);
    terrain_noise->set_fractal_lacunarity(2.0f);
    terrain_noise->set_fractal_gain(0.5f);

    // --- Temperature noise ---
    temperature_noise = Ref<FastNoiseLite>(memnew(FastNoiseLite));
    temperature_noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    temperature_noise->set_seed(seed ^ 0xDEADBEEF);  // Независимый seed
    temperature_noise->set_frequency(0.4f);            // Плавные климатические зоны
    temperature_noise->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    temperature_noise->set_fractal_octaves(3);
    temperature_noise->set_fractal_lacunarity(2.0f);
    temperature_noise->set_fractal_gain(0.4f);

    // --- Humidity noise ---
    humidity_noise = Ref<FastNoiseLite>(memnew(FastNoiseLite));
    humidity_noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    humidity_noise->set_seed(seed ^ 0xBEEFCAFE);      // Независимый seed
    humidity_noise->set_frequency(0.4f);
    humidity_noise->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    humidity_noise->set_fractal_octaves(3);
    humidity_noise->set_fractal_lacunarity(2.0f);
    humidity_noise->set_fractal_gain(0.4f);
}

float BiomeGenerator::get_density(const Vector3& world_pos) const {
    // Расстояние от центра планеты
    float dist = world_pos.length();

    // Базовое SDF сферы: > 0 — внутри, < 0 — снаружи
    float base = planet_radius - dist;

    if (dist < 1e-4f) return base; // Защита от деления на 0

    // Нормализованная позиция на поверхности (для noise-семплирования)
    Vector3 norm = world_pos / dist;
    float nx = norm.x, ny = norm.y, nz = norm.z;

    // Terrain noise — амплитуда деформации поверхности
    float terrain = terrain_noise->get_noise_3d(nx, ny, nz);
    float terrain_amplitude = planet_radius * 0.08f; // ±8% от радиуса

    // Biome noise — влияет на «грубость» рельефа
    float temp     = temperature_noise->get_noise_3d(nx, ny, nz); // [-1, 1]
    float humidity = humidity_noise->get_noise_3d(nx, ny, nz);    // [-1, 1]

    // Влажные зоны — пышный рельеф, сухие — плоский
    float biome_roughness = Math::remap(humidity, -1.0f, 1.0f, 0.5f, 1.5f);

    // Итоговая плотность
    return base + terrain * terrain_amplitude * biome_roughness;
}