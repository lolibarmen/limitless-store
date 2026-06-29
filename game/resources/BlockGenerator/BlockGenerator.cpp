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
    mountain_noise.instantiate();
    mountain_noise->set_noise_type(FastNoiseLite::TYPE_SIMPLEX_SMOOTH);
    mountain_noise->set_frequency(0.0025f);
    mountain_noise->set_fractal_octaves(4);
    mountain_noise->set_fractal_lacunarity(2.0f);
    mountain_noise->set_fractal_gain(0.5f);
}

void BlockGenerator::fill_heightmap(const Vector3i& chunk_pos, 
                                     std::vector<float>& heightmap, 
                                     int size_x, int size_z) const {
    heightmap.resize(size_x * size_z);
    for (int x = 0; x < size_x; ++x) {
        for (int z = 0; z < size_z; ++z) {
            int world_x = chunk_pos.x + x;
            int world_z = chunk_pos.z + z;
            float noise = mountain_noise->get_noise_2d(world_x, world_z);
            float height = BASE_HEIGHT + noise * MOUNTAIN_HEIGHT;
            heightmap[x + z * size_x] = height;
        }
    }
}

void BlockGenerator::init(Ref<BiomeSource> p_biome_source, int p_seed) {
    ERR_FAIL_COND_MSG(!p_biome_source.is_valid(),
        "BlockGenerator::init: biome_source невалиден");
    biome_source = p_biome_source;
    seed         = p_seed;

    setup_noise();
}

BlockData BlockGenerator::get_block(const Vector3i& world_pos) const {
    ERR_FAIL_COND_V_MSG(!biome_source.is_valid(), BlockData{},
        "BlockGenerator: не инициализирован, вызови init()");

    const float TRANSITION     = 4.0f;   // ширина перехода в блоках
    const float MOUNTAIN_HEIGHT = 40.0f; // максимальная высота гор над уровнем 0
    const float BASE_HEIGHT     = 0.0f;  // базовый уровень поверхности

    // Строим горы
    float noise_val = mountain_noise->get_noise_2d(world_pos.x, world_pos.z);
    float surface_y = BASE_HEIGHT + noise_val * MOUNTAIN_HEIGHT;

    // Signed distance: положительное значение – внутри горы, отрицательное – снаружи
    float sd = surface_y - world_pos.y;

    if (sd < -TRANSITION) return { BlockMaterial::VOID, -1.0f };

    float density = Math::clamp(sd / TRANSITION, -1.0f, 1.0f);

    BlockMaterial material = (sd > 0) ? BlockMaterial::GRASS : BlockMaterial::VOID;
    
    return { material, density };
}