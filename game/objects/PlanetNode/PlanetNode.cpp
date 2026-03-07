#include "PlanetNode.hpp"

using namespace godot;

void PlanetNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_planet_data"), &PlanetNode::get_planet_data);
    ClassDB::bind_method(D_METHOD("set_planet_data", "p_data"), &PlanetNode::set_planet_data);
    ClassDB::add_property("PlanetNode", PropertyInfo(Variant::OBJECT, "planet_data", PROPERTY_HINT_RESOURCE_TYPE, "PlanetData"), 
            "set_planet_data", "get_planet_data");

    ClassDB::bind_method(D_METHOD("spawn_initial_chunks"), &PlanetNode::spawn_initial_chunks);
}

PlanetNode::PlanetNode() {}

void PlanetNode::_ready() {
    planet_data.instantiate();

    spawn_initial_chunks();
}

void PlanetNode::spawn_initial_chunks() {

    // создаём чанки вокруг центра
    for (int x = -render_radius; x <= render_radius; x++)
    for (int y = -render_radius; y <= render_radius; y++)
    for (int z = -render_radius; z <= render_radius; z++)
    {
        ChunkNode* chunk = memnew(ChunkNode);
        
        // 🌍 мировая позиция чанка
        Vector3 world_pos =
            Vector3(x, y, z) * base_chunk_size;
            
        // 🔥 простой LOD
        // float distance = world_pos.length();
        
        int lod = 1;
        
        // if (distance > 32) lod = 8;
        // else if (distance > 16) lod = 4;
        // else if (distance > 8) lod = 2;

        int voxel_count = 8;

        // Позиция чанка в блоках 
        Vector3i chunk_coord =
            Vector3i(x, y, z) * voxel_count;
            
        chunk->configure(
            this,
            planet_data,
            chunk_coord,
            voxel_count,
            lod
        );
        chunk->set_position(
            world_pos
        );

        add_child(chunk);

        chunks.push_back(chunk);
    }
}

void PlanetNode::on_block_hit(Vector3i planet_voxel) {
    float density = planet_data->get_block(planet_voxel);
    density += 2.0;
    planet_data->set_block(planet_voxel, density);
}