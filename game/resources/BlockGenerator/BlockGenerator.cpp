#include "BlockGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

#include <cstdio>
BlockGenerator::BlockGenerator() {
    biome_generator.instantiate();
}

// ─── Утилиты ────────────────────────────────────────────────────────────────

uint32_t BlockGenerator::pos_hash(int x, int y, int z, int seed) {
    uint32_t h = static_cast<uint32_t>(seed) * 2654435761u;
    h ^= static_cast<uint32_t>(x) * 2246822519u;
    h ^= static_cast<uint32_t>(y) * 3266489917u;
    h ^= static_cast<uint32_t>(z) * 668265263u;
    h ^= h >> 17; h *= 0xbf324c81u;
    h ^= h >> 13; h *= 0x9c7493adu;
    h ^= h >> 16;
    return h;
}

float BlockGenerator::pos_random(int x, int y, int z) const {
    int seed = biome_generator.is_valid() ? biome_generator->get_seed() : 42;
    return static_cast<float>(pos_hash(x, y, z, seed)) / static_cast<float>(UINT32_MAX);
}

// ─── Логика слоёв ────────────────────────────────────────────────────────────

BlockMaterial BlockGenerator::get_surface_block(Biome biome) const {
    switch (biome) {
        case Biome::ARCTIC:
        case Biome::TUNDRA:           return BlockMaterial::SNOW;
        case Biome::TAIGA:            return BlockMaterial::SNOW;
        case Biome::DESERT:
        case Biome::SEMI_DESERT:      return BlockMaterial::SAND;
        case Biome::WASTELAND:        return BlockMaterial::GRAVEL;
        case Biome::STEPPE:
        case Biome::SAVANNA:          return BlockMaterial::DIRT;
        case Biome::SHRUBLAND:
        case Biome::DECIDUOUS_FOREST:
        case Biome::RAINFOREST:
        case Biome::TROPICAL_FOREST:
        case Biome::SUBTROPICAL_FOREST: return BlockMaterial::GRASS;
        default:                      return BlockMaterial::GRASS;
    }
}

BlockMaterial BlockGenerator::get_subsurface_block(Biome biome) const {
    switch (biome) {
        case Biome::ARCTIC:
        case Biome::TUNDRA:   return BlockMaterial::PERMAFROST;
        case Biome::DESERT:
        case Biome::SEMI_DESERT: return BlockMaterial::SAND;
        case Biome::WASTELAND:   return BlockMaterial::GRAVEL;
        default:                 return BlockMaterial::DIRT;
    }
}

BlockMaterial BlockGenerator::get_deep_block(int x, int y, int z) const {
    // Очень глубокий слой — возможна руда
    int depth = surface_level - y;

    if (depth >= ore_min_depth) {
        float r = pos_random(x, y, z);
        if (r < iron_chance)              return BlockMaterial::IRON_ORE;
        if (r < iron_chance + coal_chance) return BlockMaterial::COAL_ORE;
    }

    if (depth >= deep_threshold)          return BlockMaterial::OBSIDIAN;
    return BlockMaterial::STONE;
}

BlockMaterial BlockGenerator::get_underwater_block(Biome biome, int depth) const {
    if (depth <= 1) return BlockMaterial::SAND;
    if (biome == Biome::ARCTIC || biome == Biome::TUNDRA) return BlockMaterial::CLAY;
    return BlockMaterial::CLAY;
}

// ─── Основной метод ──────────────────────────────────────────────────────────

BlockMaterial BlockGenerator::get_block(Vector3i pos) const {
    const int x = pos.x, y = pos.y, z = pos.z;

    // Нет генератора биомов — возвращаем каменный по умолчанию
    if (!biome_generator.is_valid()) {
        if (y >= surface_level)  return BlockMaterial::AIR;
        if (y < bedrock_depth)   return BlockMaterial::STONE;
        return BlockMaterial::STONE;
    }

    // Выше поверхности — воздух или лёд/вода у моря
    if (y > surface_level)  return BlockMaterial::AIR;

    Biome biome = biome_generator->get_biome(pos);

    // Уровень моря — вода / лёд в зависимости от биома
    if (y == surface_level) {
        if (biome == Biome::ARCTIC || biome == Biome::TUNDRA)
            return BlockMaterial::ICE;
        return BlockMaterial::WATER;
    }

    // Поверхностный блок суши
    if (y == surface_level - 1)
        return get_surface_block(biome);

    // Подповерхностный слой (2–5 блоков вниз)
    const int depth = surface_level - y;
    if (depth <= 5)
        return get_subsurface_block(biome);

    // Самый нижний слой — коренная порода
    if (y < bedrock_depth)
        return BlockMaterial::STONE;

    // Всё остальное — камень / руды
    return get_deep_block(x, y, z);
}

int BlockGenerator::get_block_id(Vector3i pos) const {
    return static_cast<int>(get_block(pos));
}

String BlockGenerator::get_block_name(Vector3i pos) const {
    return material_to_string(get_block(pos));
}

String BlockGenerator::material_to_string(BlockMaterial mat) {
    switch (mat) {
        case BlockMaterial::AIR:          return "Air";
        case BlockMaterial::WATER:        return "Water";
        case BlockMaterial::ICE:          return "Ice";
        case BlockMaterial::SNOW:         return "Snow";
        case BlockMaterial::STONE:        return "Stone";
        case BlockMaterial::GRAVEL:       return "Gravel";
        case BlockMaterial::SAND:         return "Sand";
        case BlockMaterial::RED_SAND:     return "RedSand";
        case BlockMaterial::DIRT:         return "Dirt";
        case BlockMaterial::GRASS:        return "Grass";
        case BlockMaterial::CLAY:         return "Clay";
        case BlockMaterial::PERMAFROST:   return "Permafrost";
        case BlockMaterial::COAL_ORE:     return "CoalOre";
        case BlockMaterial::IRON_ORE:     return "IronOre";
        case BlockMaterial::OBSIDIAN:     return "Obsidian";
        default:                          return "Unknown";
    }
}

// ─── Биндинги GDScript ───────────────────────────────────────────────────────

void BlockGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_block_id", "pos"),   &BlockGenerator::get_block_id);
    ClassDB::bind_method(D_METHOD("get_block_name", "pos"), &BlockGenerator::get_block_name);

    ClassDB::bind_method(D_METHOD("set_biome_generator", "gen"), &BlockGenerator::set_biome_generator);
    ClassDB::bind_method(D_METHOD("get_biome_generator"),        &BlockGenerator::get_biome_generator);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "biome_generator",
        PROPERTY_HINT_RESOURCE_TYPE, "BiomeGenerator"),
        "set_biome_generator", "get_biome_generator");

    ClassDB::bind_method(D_METHOD("set_surface_level", "v"),  &BlockGenerator::set_surface_level);
    ClassDB::bind_method(D_METHOD("get_surface_level"),       &BlockGenerator::get_surface_level);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "surface_level"), "set_surface_level", "get_surface_level");

    ClassDB::bind_method(D_METHOD("set_bedrock_depth", "v"), &BlockGenerator::set_bedrock_depth);
    ClassDB::bind_method(D_METHOD("get_bedrock_depth"),      &BlockGenerator::get_bedrock_depth);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "bedrock_depth"), "set_bedrock_depth", "get_bedrock_depth");

    ClassDB::bind_method(D_METHOD("set_ore_min_depth", "v"), &BlockGenerator::set_ore_min_depth);
    ClassDB::bind_method(D_METHOD("get_ore_min_depth"),      &BlockGenerator::get_ore_min_depth);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "ore_min_depth"), "set_ore_min_depth", "get_ore_min_depth");

    ClassDB::bind_method(D_METHOD("set_deep_threshold", "v"), &BlockGenerator::set_deep_threshold);
    ClassDB::bind_method(D_METHOD("get_deep_threshold"),      &BlockGenerator::get_deep_threshold);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "deep_threshold"), "set_deep_threshold", "get_deep_threshold");

    ClassDB::bind_method(D_METHOD("set_coal_chance", "v"), &BlockGenerator::set_coal_chance);
    ClassDB::bind_method(D_METHOD("get_coal_chance"),      &BlockGenerator::get_coal_chance);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "coal_chance"), "set_coal_chance", "get_coal_chance");

    ClassDB::bind_method(D_METHOD("set_iron_chance", "v"), &BlockGenerator::set_iron_chance);
    ClassDB::bind_method(D_METHOD("get_iron_chance"),      &BlockGenerator::get_iron_chance);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "iron_chance"), "set_iron_chance", "get_iron_chance");
}