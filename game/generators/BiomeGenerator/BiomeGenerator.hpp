#pragma once
#include <godot_cpp/classes/fast_noise_lite.hpp>

namespace godot {

struct ChunkRequest {
    Vector3i origin;
    Vector3  position;   // world-space центр, был slot.position
    int      lod;
    int      seed;
};

// Отвечает на два вопроса:
//   1. get_density(pos)  → float  — есть ли тут порода?
//   2. get_biome_id(pos) → int    — какой биом?
//
// query() возвращает слоты для ChunkManager —
// грубую сетку чанков вокруг наблюдателя.
class BiomeGenerator : public Resource {
    GDCLASS(BiomeGenerator, Resource)

private:
    int   seed          = 0;
    float planet_radius = 100.0f;

    Ref<FastNoiseLite> terrain_noise;
    Ref<FastNoiseLite> temperature_noise;
    Ref<FastNoiseLite> humidity_noise;

protected:
    static void _bind_methods();

public:
    BiomeGenerator() = default;

    // Инициализация из контекста планеты
    void initialize(int p_seed, float p_radius);

    // === ГЛАВНЫЕ МЕТОДЫ ===

    // Плотность в точке: > 0 порода, < 0 воздух
    float get_density(const Vector3& world_pos) const;

    // ID биома в точке
    int get_biome_id(const Vector3& world_pos) const;

    // IGenerator::query — возвращает слоты чанков вокруг наблюдателя
    // Каждый Slot.extra = { origin: Vector3i, lod: int }
    std::vector<Slot> query(const Vector3& camera_pos, float radius) const;

    // Геттеры/сеттеры для инспектора Godot
    void  set_seed(int p)          { seed = p; }
    int   get_seed() const         { return seed; }
    void  set_planet_radius(float p) { planet_radius = p; }
    float get_planet_radius() const  { return planet_radius; }
};

} // namespace godot