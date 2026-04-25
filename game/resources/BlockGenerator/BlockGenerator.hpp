#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <GameStructs/Blocks.hpp>
#include <BiomeSource/BiomeSource.hpp>

namespace godot {

class BlockGenerator : public RefCounted {
    GDCLASS(BlockGenerator, RefCounted)

private:
    int seed = 0;

    Ref<BiomeSource>   biome_source;
    Ref<FastNoiseLite> noise_cave, noise_detail, noise_ridge;

    float cave_scale   = 0.04f;
    float detail_scale = 0.02f;

    void setup_noise();

    BlockData classify(
        const Vector3i&  world_pos,
        const BiomeData& biome,
        float            cave,
        float            detail,
        float            ridgel
    ) const;

protected:
    static void _bind_methods();

public:
    BlockGenerator() = default;
    ~BlockGenerator() override = default;

    // Явная инициализация — вызывается из менеджера
    void init(Ref<BiomeSource> p_biome_source, int p_seed);

    BlockData get_block(const Vector3i& world_pos) const;

    int  get_seed() const { return seed; }
    void set_seed(int v)  { seed = v; setup_noise(); }
};

} // namespace godot