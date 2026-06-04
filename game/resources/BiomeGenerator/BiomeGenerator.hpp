#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <GameStructs/Biomes.hpp>

namespace godot {

class BiomeGenerator : public RefCounted {
    GDCLASS(BiomeGenerator, RefCounted)

private:
    int seed = 0;

    void setup_noise();

    BiomeType classify(float humidity, float temperature, float height) const;

protected:
    static void _bind_methods();

public:
    BiomeGenerator() = default;
    ~BiomeGenerator() override = default;

    void init(int p_seed);

    BiomeData get_biome(const Vector3i& world_pos) const;

    int get_seed() const { return seed; }
    void set_seed(int p_seed) { seed = p_seed; }
};

} // namespace godot