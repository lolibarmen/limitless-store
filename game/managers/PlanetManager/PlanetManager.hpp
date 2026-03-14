#pragma once
#include <resources/PlanetContext/PlanetContext.hpp>
#include <resources/PlanetData/PlanetData.hpp>
#include <generators/BiomeGenerator/BiomeGenerator.hpp>
#include <managers/ChunkManager/ChunkManager.hpp>

namespace godot {

class PlanetManager : public Resource {
    GDCLASS(PlanetManager, Resource)

private:
    PlanetContext       context;
    Ref<PlanetData>     planet_data;
    Ref<BiomeGenerator> biome_generator;
    ChunkManager*       chunk_manager = nullptr;

protected:
    static void _bind_methods();

public:
    PlanetManager() = default;

    void initialize(const PlanetContext& p_context);

    void _ready()                          override;
    void update(const Vector3& camera_pos);

    void action(const Vector3& world_pos, float radius, float delta);
};

} // namespace godot