#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <PlanetData/PlanetData.hpp>
#include <ChunkNode/ChunkNode.hpp>
#include <BlockGenerator/BlockGenerator.hpp>

namespace godot {

class ChunkManager : public Node3D {
    GDCLASS(ChunkManager, Node3D)

private:
    Ref<PlanetData>     planet_data;
    Ref<BlockGenerator> block_generator;

    std::unordered_map<int64_t, ChunkNode*> chunks;

    int compute_lod(float distance);
    static int64_t chunk_hash(Vector3i pos, int lod);

    void populate_chunk_data(Vector3i chunk_origin, int voxel_count, int lod);

protected:
    static void _bind_methods();
    
    public:
    ChunkManager();
    ~ChunkManager() override = default;
    
    void _ready() override;
    void _process(double delta) override;
    
    void update();

    void action(const Vector3& world_pos, float radius, float delta);

    ChunkNode* get_chunk_by_origin(const Vector3i& origin) const;
    
    Ref<PlanetData> get_planet_data() { return planet_data; }
    void set_planet_data(Ref<PlanetData> p_data) { planet_data = p_data; }
};

} // namespace godot