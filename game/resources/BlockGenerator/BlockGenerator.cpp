#include "BlockGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void BlockGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_seed"),      &BlockGenerator::get_seed);
    ClassDB::bind_method(D_METHOD("set_seed", "v"), &BlockGenerator::set_seed);
    ClassDB::add_property("BlockGenerator",
        PropertyInfo(Variant::INT, "seed"),
        "set_seed", "get_seed");
}

void BlockGenerator::setup_noise() {
    noise_cave.instantiate();
    noise_cave->set_seed(seed + 10);
    noise_cave->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_cave->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    noise_cave->set_fractal_octaves(4);
    noise_cave->set_fractal_lacunarity(2.0f);
    noise_cave->set_fractal_gain(0.5f);
    noise_cave->set_frequency(cave_scale);

    noise_detail.instantiate();
    noise_detail->set_seed(seed + 20);
    noise_detail->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_detail->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    noise_detail->set_fractal_octaves(6);
    noise_detail->set_fractal_lacunarity(2.0f);
    noise_detail->set_fractal_gain(0.45f);
    noise_detail->set_frequency(detail_scale);

    noise_ridge.instantiate();
    noise_ridge->set_seed(seed + 30);
    noise_ridge->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_ridge->set_fractal_type(FastNoiseLite::FRACTAL_RIDGED);
    noise_ridge->set_fractal_octaves(5);
    noise_ridge->set_frequency(detail_scale * 0.6f);
}

void BlockGenerator::init(Ref<BiomeSource> p_biome_source, int p_seed) {
    ERR_FAIL_COND_MSG(!p_biome_source.is_valid(),
        "BlockGenerator::init: biome_source невалиден");

    biome_source = p_biome_source;
    seed         = p_seed;
    setup_noise();
}

// Smooth hermite interpolation
static inline float smoothstep(float edge0, float edge1, float x) {
    float t = Math::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t;
}

BlockData BlockGenerator::classify(
    const Vector3i&  world_pos,
    const BiomeData& biome,
    float            cave,
    float            detail,
    float            ridge
) const {
    const Vector3 PLANET_CENTER = Vector3(0, 0, 0);
    const float   PLANET_RADIUS = 128.0f;

    Vector3 pos_f = Vector3(world_pos.x, world_pos.y, world_pos.z);
    float dist = (pos_f - PLANET_CENTER).length();

    // Горы через ridge шум, сглаженные по высоте биома
    float mountain_factor = smoothstep(0.5f, 1.0f, biome.height);
    float height_offset = biome.height * 48.0f
                        + detail * 12.0f
                        + ridge * 24.0f * mountain_factor;

    float surface_r = PLANET_RADIUS + height_offset;

    if (dist > surface_r + 1.0f)
        return { BlockMaterial::AIR, -1.0f };

    // Плавная плотность у границы поверхности
    float surface_dist = surface_r - dist;
    float density = smoothstep(-1.0f, 1.0f, surface_dist);

    if (surface_dist < -0.5f)
        return { BlockMaterial::AIR, -1.0f };

    // // Пещеры: плавный порог, нет пещер у самой поверхности
    float surface_fade  = smoothstep(0.0f, 6.0f, surface_dist);
    float cave_threshold = 0.50f + (1.0f - surface_fade) * 0.25f;
    float cave_abs = Math::abs(cave);
    if (cave_abs > cave_threshold) {
        float cave_density = smoothstep(cave_threshold + 0.05f, cave_threshold, cave_abs);
        if (cave_density < 0.01f)
            return { BlockMaterial::AIR, -1.0f };
        density = Math::min(density, cave_density);
    }

    float depth = surface_dist;

    BlockMaterial mat;
    switch (biome.type) {
    case BiomeType::OCEAN:
        if (depth < 1.0f) mat = BlockMaterial::WATER;
        else if (depth < 4.0f) mat = BlockMaterial::GRAVEL;
        else mat = BlockMaterial::STONE;
        break;
    case BiomeType::DESERT:
        if (depth < 4.0f) mat = BlockMaterial::SAND;
        else mat = BlockMaterial::STONE;
        break;
    case BiomeType::TUNDRA:
        if (depth < 1.0f) mat = BlockMaterial::SNOW;
        else if (depth < 3.0f) mat = BlockMaterial::DIRT;
        else mat = BlockMaterial::STONE;
        break;
    case BiomeType::MOUNTAIN:
        if (depth < 1.0f && biome.height < 0.8f) mat = BlockMaterial::SNOW;
        else mat = BlockMaterial::STONE;
        break;
    case BiomeType::FOREST:
    case BiomeType::PLAINS:
    default:
        if (depth < 1.0f) mat = BlockMaterial::GRASS;
        else if (depth < 4.0f) mat = BlockMaterial::DIRT;
        else mat = BlockMaterial::STONE;
        break;
    }

    return { mat, density };
}

// BlockData BlockGenerator::classify(const Vector3i& world_pos, const BiomeData& biome,
//                                    float cave, float detail, float ridge) const {
//     const Vector3 PLANET_CENTER = Vector3(0, 0, 0);
//     const float   PLANET_RADIUS = 128.0f;

//     Vector3 pos_f = Vector3(world_pos.x, world_pos.y, world_pos.z);
//     float dist = (pos_f - PLANET_CENTER).length();

//     // density > 0  — внутри сферы, density < 0 — снаружи
//     // +1.0 = центр планеты, 0.0 = ровно на поверхности, -1.0 = на 1 блок снаружи
//     float density = PLANET_RADIUS - dist;

//     if (density < 0.0f)
//         return { BlockMaterial::AIR, density };

//     BlockMaterial mat = BlockMaterial::STONE;
//     return { mat, density };
// }

BlockData BlockGenerator::get_block(const Vector3i& world_pos) const {
    ERR_FAIL_COND_V_MSG(!biome_source.is_valid(), BlockData{},
        "BlockGenerator: не инициализирован, вызови init()");
    ERR_FAIL_COND_V_MSG(!noise_cave.is_valid() || !noise_detail.is_valid() || !noise_ridge.is_valid(), BlockData{},
        "BlockGenerator: шумы не инициализированы");

    BiomeData biome = biome_source->get_biome(world_pos);
    float cave      = noise_cave->get_noise_3d(world_pos.x, world_pos.y, world_pos.z);
    float detail    = noise_detail->get_noise_2d(world_pos.x, world_pos.z);
    float ridge     = noise_ridge->get_noise_2d(world_pos.x, world_pos.z);

    return classify(world_pos, biome, cave, detail, ridge);
}