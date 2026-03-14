#pragma once
#include <resources/PlanetData/PlanetData.hpp>
#include <generators/BiomeGenerator/BiomeGenerator.hpp>
#include <objects/ChunkNode/ChunkNode.hpp>
#include <unordered_map>

namespace godot {

class ChunkManager : public Resource {
    GDCLASS(ChunkManager, Resource)

private:
    Ref<PlanetData>     planet_data;
    Ref<BiomeGenerator> biome_generator;

    std::unordered_map<int64_t, ChunkNode*> chunks;

    static int64_t chunk_hash(const Vector3i& pos, int lod);
    int  compute_lod(float distance) const;
    void spawn_chunk(const Slot& slot);
    void despawn_chunk(int64_t hash);

protected:
    static void _bind_methods() {};

public:
    ChunkManager() = default;

    void initialize(Ref<PlanetData> p_data, Ref<BiomeGenerator> p_generator);

    void update(const Vector3& camera_pos);

    void action(const Vector3& world_pos, float radius, float delta);

    ChunkNode* get_chunk_by_origin(const Vector3i& origin) const;
};

} // namespace godot