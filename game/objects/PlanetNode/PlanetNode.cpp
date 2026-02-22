#include "PlanetNode.h"

using namespace godot;

void PlanetNode::_bind_methods() {}

PlanetNode::PlanetNode() {
    
}

void PlanetNode::_ready() {
    planet_data.instantiate();

    float density = planet_data->get_block(Vector3i(0,0,0));

    ChunkNode* new_chunk = memnew(ChunkNode);
    new_chunk->set_position(Vector3(0.0, 0.0, 0.0));
    new_chunk->set_planet_data(planet_data);
    add_child(new_chunk);

    chunks.push_back(new_chunk);
}