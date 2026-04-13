#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <GameStructs/Biomes.hpp>

namespace godot {

class BiomeGenerator : public RefCounted {
    GDCLASS(BiomeGenerator, RefCounted)

private:
    int seed = 0;

    Ref<FastNoiseLite> noise_humidity;
    Ref<FastNoiseLite> noise_temperature;
    Ref<FastNoiseLite> noise_height;

    float humidity_scale    = 0.003f;
    float temperature_scale = 0.002f;
    float height_scale      = 0.005f;

    void setup_noise();

    BiomeType classify(float humidity, float temperature, float height) const;

protected:
    static void _bind_methods();

public:
    BiomeGenerator() = default;
    ~BiomeGenerator() override = default;

    void init(int p_seed);

    BiomeData get_biome(const Vector3i& world_pos) const;

    int  get_seed() const { return seed; }
};

} // namespace godot