#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <PlanetData/PlanetData.hpp>
#include <ChunkNode/ChunkNode.hpp>

namespace godot {

class PlanetNode : public Node3D {
    GDCLASS(PlanetNode, Node3D)

private:
    Ref<PlanetData> planet_data;

    std::unordered_map<int64_t, ChunkNode*> chunks;

    int compute_lod(float distance);
    static int64_t chunk_hash(Vector3i pos, int lod);

protected:
    static void _bind_methods();
    
    public:
    PlanetNode();
    ~PlanetNode() override = default;
    
    void _ready() override;
    void _process(double delta) override;
    
    void update_chunks();
    
    void on_block_hit(Vector3i planet_voxel);
    
    Ref<PlanetData> get_planet_data() { return planet_data; }
    void set_planet_data(Ref<PlanetData> p_data) { planet_data = p_data; }
};

} // namespace godot