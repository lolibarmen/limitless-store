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
    noise_cave->set_frequency(cave_scale);

    noise_detail.instantiate();
    noise_detail->set_seed(seed + 20);
    noise_detail->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    noise_detail->set_frequency(detail_scale);
}

void BlockGenerator::init(Ref<BiomeSource> p_biome_source, int p_seed) {
    ERR_FAIL_COND_MSG(!p_biome_source.is_valid(),
        "BlockGenerator::init: biome_source невалиден");

    biome_source = p_biome_source;
    seed         = p_seed;
    setup_noise();
}

BlockData BlockGenerator::classify(
    const Vector3i&  world_pos,
    const BiomeData& biome,
    float            cave,
    float            detail
) const {
    float surface_y = biome.height * 64.0f + detail * 8.0f;

    float cave_threshold = 0.55f - Math::max(0.0f, (float)world_pos.y * 0.001f);
    if (Math::abs(cave) > cave_threshold && world_pos.y < (int)surface_y)
        return { BlockMaterial::AIR, -1.0f };

    if ((float)world_pos.y > surface_y)
        return { BlockMaterial::AIR, -1.0f };

    float depth = surface_y - (float)world_pos.y;

    float density = (depth < 1.0f) ? Math::fmod(surface_y, 1.0f) : 1.0f;

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

BlockData BlockGenerator::get_block(const Vector3i& world_pos) const {
    ERR_FAIL_COND_V_MSG(!biome_source.is_valid(), BlockData{},
        "BlockGenerator: не инициализирован, вызови init()");
    ERR_FAIL_COND_V_MSG(!noise_cave.is_valid() || !noise_detail.is_valid(), BlockData{},
        "BlockGenerator: шумы не инициализированы");

    BiomeData biome = biome_source->get_biome(world_pos);
    float cave      = noise_cave->get_noise_3d(world_pos.x, world_pos.y, world_pos.z);
    float detail    = noise_detail->get_noise_2d(world_pos.x, world_pos.z);

    return classify(world_pos, biome, cave, detail);
}