#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <unordered_map>
#include <GameStructs/Biomes.hpp>
#include <BiomeGenerator/BiomeGenerator.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <Utils/SpatialHash.hpp>

namespace godot {

class BiomeSource : public Resource {
    GDCLASS(BiomeSource, Resource)

private:
    Ref<BiomeGenerator> generator;
    std::unordered_map<Vector3i, BiomeData, Vector3iHash> edits;

protected:
    static void _bind_methods();

public:
    BiomeSource() = default;
    ~BiomeSource() override = default;

    BiomeData get_biome(const Vector3i& world_pos) const;
    void      set_biome(const Vector3i& world_pos, const BiomeData& data);
    void      reset_biome(const Vector3i& world_pos);
    bool      has_edit(const Vector3i& world_pos) const;

    Ref<BiomeGenerator> get_generator() const { return generator; }
    void                set_generator(Ref<BiomeGenerator> g) { generator = g; }
};

} // namespace godot