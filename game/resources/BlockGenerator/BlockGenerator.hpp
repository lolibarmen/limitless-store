#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/string.hpp>
#include <BiomeGenerator/BiomeGenerator.hpp>

namespace godot {

enum class BlockMaterial : uint8_t {
    AIR,
    WATER,
    ICE,
    SNOW,
    STONE,
    GRAVEL,
    SAND,
    RED_SAND,
    DIRT,
    GRASS,
    CLAY,
    PERMAFROST,
    COAL_ORE,
    IRON_ORE,
    OBSIDIAN,
    COUNT
};

class BlockGenerator : public Resource {
    GDCLASS(BlockGenerator, Resource)

private:
    Ref<BiomeGenerator> biome_generator;

    // Высотные пороги поверхности
    int surface_level     = 64;   // уровень моря / поверхности
    int bedrock_depth     = 5;    // глубина до коренной породы
    int ore_min_depth     = 10;   // минимальная глубина появления руды
    int deep_threshold    = 30;   // глубина "глубокого" слоя

    // Шанс появления руды (0.0 - 1.0)
    float coal_chance = 0.05f;
    float iron_chance = 0.02f;

    // Простой хэш для псевдо-случайных решений на позиции
    static uint32_t pos_hash(int x, int y, int z, int seed);
    float pos_random(int x, int y, int z) const;

    BlockMaterial get_surface_block(Biome biome) const;
    BlockMaterial get_subsurface_block(Biome biome) const;
    BlockMaterial get_deep_block(int x, int y, int z) const;
    BlockMaterial get_underwater_block(Biome biome, int depth) const;

protected:
    static void _bind_methods();

public:
    BlockGenerator();

    // Основной метод — возвращает материал блока по координатам
    BlockMaterial get_block(Vector3i pos) const;
    int           get_block_id(Vector3i pos) const;
    String        get_block_name(Vector3i pos) const;

    static String material_to_string(BlockMaterial mat);

    // Сеттеры/геттеры BiomeGenerator
    void set_biome_generator(const Ref<BiomeGenerator>& gen) { biome_generator = gen; }
    Ref<BiomeGenerator> get_biome_generator() const { return biome_generator; }

    void  set_surface_level(int v)   { surface_level = v; }
    int   get_surface_level() const  { return surface_level; }
    void  set_bedrock_depth(int v)   { bedrock_depth = v; }
    int   get_bedrock_depth() const  { return bedrock_depth; }
    void  set_ore_min_depth(int v)   { ore_min_depth = v; }
    int   get_ore_min_depth() const  { return ore_min_depth; }
    void  set_deep_threshold(int v)  { deep_threshold = v; }
    int   get_deep_threshold() const { return deep_threshold; }
    void  set_coal_chance(float v)   { coal_chance = v; }
    float get_coal_chance() const    { return coal_chance; }
    void  set_iron_chance(float v)   { iron_chance = v; }
    float get_iron_chance() const    { return iron_chance; }
};

} // namespace godot