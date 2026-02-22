#include "PlanetNode.hpp"

using namespace godot;

void PlanetNode::_bind_methods() {}

PlanetNode::PlanetNode() {
    
}

void PlanetNode::_ready() {
    planet_data.instantiate();

    Vector3i world_size = planet_data->get_world_size();
    int chunks_size = ChunkNode::CHUNK_SIZE;

    Vector3 planet_size = ChunkNode::BLOCK_SIZE * world_size;

    Vector3i chunks_count = world_size / chunks_size;

    for(int x = 0; x < chunks_count.x; x++)
    for(int y = 0; y < chunks_count.y; y++)
    for(int z = 0; z < chunks_count.z; z++)
    {
        ChunkNode* new_chunk = memnew(ChunkNode);

        Vector3 chunk_pos = Vector3(x,y,z) * chunks_size - planet_size / 2;

        new_chunk->set_position(chunk_pos);
        new_chunk->set_planet_data(planet_data);

        add_child(new_chunk);

        chunks.push_back(new_chunk);
    }
}