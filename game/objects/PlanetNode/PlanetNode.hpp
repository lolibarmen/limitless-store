#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <vector>

#include <PlanetData/PlanetData.hpp>
#include <ChunkNode/ChunkNode.hpp>


namespace godot {

class PlanetNode : public Node3D {
    GDCLASS(PlanetNode, Node3D)

private:
    // Чанки
    std::vector<ChunkNode*> chunks;

    // Менеджер данных о планете
    Ref<PlanetData> planet_data;

protected:
    static void _bind_methods();

public:
    PlanetNode();
    ~PlanetNode() override = default;
    
    void _ready() override;
};

} // namespace godot