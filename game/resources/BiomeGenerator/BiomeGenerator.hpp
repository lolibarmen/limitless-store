#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/core/math.hpp>

namespace godot {

enum class Biome : uint8_t {
    ARCTIC, TUNDRA, TAIGA, WASTELAND, STEPPE,
    SHRUBLAND, DECIDUOUS_FOREST, RAINFOREST,
    TROPICAL_FOREST, SEMI_DESERT, SAVANNA,
    SUBTROPICAL_FOREST, DESERT,
    COUNT
};

class BiomeGenerator : public Resource {
    GDCLASS(BiomeGenerator, Resource)

private:
    int   seed        = 42;
    float scale       = 0.02f;   // масштаб для блоков (был 0.003 для чанков)
    int   octaves     = 4;
    float persistence = 0.5f;
    float lacunarity  = 2.0f;

    // Высотные пороги для учёта Y-координаты
    int height_sea_level    = 64;
    int height_mountain     = 128;
    int height_high_mountain = 192;

    static constexpr float TEMP_OFFSET_X = 0.0f;
    static constexpr float TEMP_OFFSET_Y = 0.0f;
    static constexpr float HUM_OFFSET_X  = 31337.0f;
    static constexpr float HUM_OFFSET_Y  = 9999.0f;

    static uint32_t fnv1a(uint32_t x, uint32_t y, uint32_t s);
    float value_noise(float x, float y, uint32_t s) const;
    float fbm(float x, float y, uint32_t s) const;

    // Модификатор температуры по высоте (выше = холоднее)
    float height_temp_modifier(int y) const;

    static Biome whittaker_table(int temp_idx, int hum_idx);

protected:
    static void _bind_methods();

public:
    BiomeGenerator() = default;

    Biome   get_biome(Vector3i block) const;
    int     get_biome_id(Vector3i block) const;
    String  get_biome_name(Vector3i block) const;
    Vector2 get_raw_values(Vector3i block) const;  // возвращает (temp, hum) с учётом высоты

    void  set_seed(int p)               { seed = p; }
    int   get_seed() const              { return seed; }
    void  set_scale(float p)            { scale = p; }
    float get_scale() const             { return scale; }
    void  set_octaves(int p)            { octaves = Math::clamp(p, 1, 8); }
    int   get_octaves() const           { return octaves; }
    void  set_persistence(float p)      { persistence = Math::clamp(p, 0.1f, 1.0f); }
    float get_persistence() const       { return persistence; }
    void  set_lacunarity(float p)       { lacunarity = Math::clamp(p, 1.0f, 4.0f); }
    float get_lacunarity() const        { return lacunarity; }

    void set_height_sea_level(int p)         { height_sea_level = p; }
    int  get_height_sea_level() const        { return height_sea_level; }
    void set_height_mountain(int p)          { height_mountain = p; }
    int  get_height_mountain() const         { return height_mountain; }
    void set_height_high_mountain(int p)     { height_high_mountain = p; }
    int  get_height_high_mountain() const    { return height_high_mountain; }
};

} // namespace godot