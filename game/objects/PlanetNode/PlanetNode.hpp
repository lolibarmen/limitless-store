#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <vector>

#include <PlanetData/PlanetData.hpp>
#include <ChunkNode/ChunkNode.hpp>

namespace godot {

class PlanetNode : public Node3D {
    GDCLASS(PlanetNode, Node3D)

private:
    Ref<PlanetData> planet_data;

    std::vector<ChunkNode*> chunks;

    // LOD параметры
    int render_radius = 1;     // сколько чанков вокруг центра
    float base_chunk_size = 8.0f;

protected:
    static void _bind_methods();

    void spawn_initial_chunks();

public:
    PlanetNode();
    ~PlanetNode() override = default;

    void _ready() override;
};

} // namespace godot