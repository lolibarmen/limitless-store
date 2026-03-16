#include "BiomeGenerator.hpp"
#include <cmath>

namespace godot {

uint32_t BiomeGenerator::fnv1a(uint32_t x, uint32_t y, uint32_t s) {
    constexpr uint32_t FNV_PRIME  = 16777619u;
    constexpr uint32_t FNV_OFFSET = 2166136261u;
    uint32_t h = FNV_OFFSET;
    h ^= x; h *= FNV_PRIME;
    h ^= y; h *= FNV_PRIME;
    h ^= s; h *= FNV_PRIME;
    return h;
}

float BiomeGenerator::value_noise(float x, float y, uint32_t s) const {
    int ix = (int)std::floor(x);
    int iy = (int)std::floor(y);
    float fx = x - ix;
    float fy = y - iy;

    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    auto rnd = [&](int cx, int cy) -> float {
        uint32_t h = fnv1a((uint32_t)cx, (uint32_t)cy, s);
        return (float)(h & 0xFFFF) / 65535.0f;
    };

    float a = rnd(ix,   iy);
    float b = rnd(ix+1, iy);
    float c = rnd(ix,   iy+1);
    float d = rnd(ix+1, iy+1);

    return a + (b-a)*ux + (c-a)*uy + (a-b-c+d)*ux*uy;
}

float BiomeGenerator::fbm(float x, float y, uint32_t s) const {
    float value     = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float max_value = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        value     += value_noise(x * frequency, y * frequency, s + i * 997) * amplitude;
        max_value += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return value / max_value;
}

// Чем выше блок — тем ниже температура.
// Возвращает значение в [-1, 0]: прибавляется к базовой температуре.
float BiomeGenerator::height_temp_modifier(int y) const {
    if (y <= height_sea_level) {
        return 0.0f;                                    // ниже уровня моря — без штрафа
    } else if (y <= height_mountain) {
        // плавное охлаждение от 0 до -0.3
        float t = (float)(y - height_sea_level) /
                  (float)(height_mountain - height_sea_level);
        return -0.3f * t;
    } else if (y <= height_high_mountain) {
        // охлаждение от -0.3 до -0.6
        float t = (float)(y - height_mountain) /
                  (float)(height_high_mountain - height_mountain);
        return -0.3f - 0.3f * t;
    } else {
        return -0.6f;                                   // выше — всегда арктика
    }
}

Biome BiomeGenerator::whittaker_table(int temp_idx, int hum_idx) {
    static const Biome TABLE[4][5] = {
        // temp:  холодно       прохладно      умеренно              тепло                  жарко
        /* сухо */ { Biome::ARCTIC,  Biome::WASTELAND,  Biome::SEMI_DESERT,    Biome::DESERT,             Biome::DESERT          },
        /* ср.  */ { Biome::ARCTIC,  Biome::STEPPE,     Biome::SHRUBLAND,      Biome::SEMI_DESERT,        Biome::SAVANNA         },
        /* влажно*/{ Biome::TUNDRA,  Biome::TAIGA,      Biome::DECIDUOUS_FOREST, Biome::SUBTROPICAL_FOREST, Biome::SAVANNA       },
        /* оч.вл*/{ Biome::TUNDRA,  Biome::TAIGA,       Biome::RAINFOREST,     Biome::TROPICAL_FOREST,   Biome::TROPICAL_FOREST },
    };
    return TABLE[hum_idx][temp_idx];
}

Biome BiomeGenerator::get_biome(Vector3i block) const {
    // Шум семплируется по X/Z — горизонтальные координаты
    float bx = (float)block.x * scale;
    float bz = (float)block.z * scale;

    float temp = fbm(bx + TEMP_OFFSET_X, bz + TEMP_OFFSET_Y, (uint32_t)seed);
    float hum  = fbm(bx + HUM_OFFSET_X,  bz + HUM_OFFSET_Y,  (uint32_t)(seed ^ 0xDEADBEEF));

    // Применяем высотный штраф к температуре и клампируем в [0, 1]
    temp = Math::clamp(temp + height_temp_modifier(block.y), 0.0f, 1.0f);

    int temp_idx = Math::clamp((int)(temp * 4.99f), 0, 4);
    int hum_idx  = Math::clamp((int)(hum  * 3.99f), 0, 3);

    return whittaker_table(temp_idx, hum_idx);
}

int BiomeGenerator::get_biome_id(Vector3i block) const {
    return (int)get_biome(block);
}

String BiomeGenerator::get_biome_name(Vector3i block) const {
    static const char* NAMES[] = {
        "Arctic", "Tundra", "Taiga", "Wasteland", "Steppe",
        "Shrubland", "Deciduous Forest", "Rainforest",
        "Tropical Forest", "Semi-Desert", "Savanna",
        "Subtropical Forest", "Desert"
    };
    return String(NAMES[(int)get_biome(block)]);
}

Vector2 BiomeGenerator::get_raw_values(Vector3i block) const {
    float bx = (float)block.x * scale;
    float bz = (float)block.z * scale;
    float temp = fbm(bx + TEMP_OFFSET_X, bz + TEMP_OFFSET_Y, (uint32_t)seed);
    float hum  = fbm(bx + HUM_OFFSET_X,  bz + HUM_OFFSET_Y,  (uint32_t)(seed ^ 0xDEADBEEF));
    temp = Math::clamp(temp + height_temp_modifier(block.y), 0.0f, 1.0f);
    return Vector2(temp, hum);
}

void BiomeGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_biome_id",   "block"), &BiomeGenerator::get_biome_id);
    ClassDB::bind_method(D_METHOD("get_biome_name", "block"), &BiomeGenerator::get_biome_name);
    ClassDB::bind_method(D_METHOD("get_raw_values", "block"), &BiomeGenerator::get_raw_values);

    ClassDB::bind_method(D_METHOD("set_seed",        "seed"),        &BiomeGenerator::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"),                       &BiomeGenerator::get_seed);
    ClassDB::bind_method(D_METHOD("set_scale",       "scale"),       &BiomeGenerator::set_scale);
    ClassDB::bind_method(D_METHOD("get_scale"),                      &BiomeGenerator::get_scale);
    ClassDB::bind_method(D_METHOD("set_octaves",     "octaves"),     &BiomeGenerator::set_octaves);
    ClassDB::bind_method(D_METHOD("get_octaves"),                    &BiomeGenerator::get_octaves);
    ClassDB::bind_method(D_METHOD("set_persistence", "persistence"), &BiomeGenerator::set_persistence);
    ClassDB::bind_method(D_METHOD("get_persistence"),                &BiomeGenerator::get_persistence);
    ClassDB::bind_method(D_METHOD("set_lacunarity",  "lacunarity"),  &BiomeGenerator::set_lacunarity);
    ClassDB::bind_method(D_METHOD("get_lacunarity"),                 &BiomeGenerator::get_lacunarity);

    ClassDB::bind_method(D_METHOD("set_height_sea_level",      "h"), &BiomeGenerator::set_height_sea_level);
    ClassDB::bind_method(D_METHOD("get_height_sea_level"),           &BiomeGenerator::get_height_sea_level);
    ClassDB::bind_method(D_METHOD("set_height_mountain",       "h"), &BiomeGenerator::set_height_mountain);
    ClassDB::bind_method(D_METHOD("get_height_mountain"),            &BiomeGenerator::get_height_mountain);
    ClassDB::bind_method(D_METHOD("set_height_high_mountain",  "h"), &BiomeGenerator::set_height_high_mountain);
    ClassDB::bind_method(D_METHOD("get_height_high_mountain"),       &BiomeGenerator::get_height_high_mountain);

    ADD_PROPERTY(PropertyInfo(Variant::INT,   "seed"),        "set_seed",        "get_seed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale"),       "set_scale",       "get_scale");
    ADD_PROPERTY(PropertyInfo(Variant::INT,   "octaves"),     "set_octaves",     "get_octaves");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "persistence"), "set_persistence", "get_persistence");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lacunarity"),  "set_lacunarity",  "get_lacunarity");

    ADD_GROUP("Height Thresholds", "height_");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "height_sea_level"),      "set_height_sea_level",      "get_height_sea_level");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "height_mountain"),       "set_height_mountain",       "get_height_mountain");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "height_high_mountain"),  "set_height_high_mountain",  "get_height_high_mountain");
}

} // namespace godot