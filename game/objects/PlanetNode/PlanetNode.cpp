#include "PlanetNode.hpp"

#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>

using namespace godot;

void PlanetNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_planet_data"), &PlanetNode::get_planet_data);
    ClassDB::bind_method(D_METHOD("set_planet_data", "p_data"), &PlanetNode::set_planet_data);
    ClassDB::add_property("PlanetNode", PropertyInfo(Variant::OBJECT, "planet_data", PROPERTY_HINT_RESOURCE_TYPE, "PlanetData"), 
            "set_planet_data", "get_planet_data");

    ClassDB::bind_method(D_METHOD("update_chunks"), &PlanetNode::update_chunks);
}

PlanetNode::PlanetNode() {}

void PlanetNode::_ready() {
    planet_data.instantiate();

    update_chunks();
}

int PlanetNode::compute_lod(float distance) {

    if (distance < 64) return 1;
    if (distance < 128) return 2;
    if (distance < 256) return 4;
    if (distance < 512) return 8;

    return 16;
}

void PlanetNode::_process(double delta) {
    update_chunks();
}

int64_t PlanetNode::chunk_hash(Vector3i pos, int lod) {
    int64_t h = 1469598103934665603ULL;

    h ^= pos.x; h *= 1099511628211ULL;
    h ^= pos.y; h *= 1099511628211ULL;
    h ^= pos.z; h *= 1099511628211ULL;
    h ^= lod;   h *= 1099511628211ULL;

    return h;
}

void PlanetNode::update_chunks() {
    Vector3 viewer = get_viewport()->get_camera_3d()->get_global_position();
    int range = 64;
    int voxel_count = 8;

    // Собираем ключи всех чанков, которые должны существовать в этом кадре
    std::unordered_set<int64_t> active_keys;

    for (int x = -range; x < range; x += voxel_count)
    for (int y = -range; y < range; y += voxel_count)
    for (int z = -range; z < range; z += voxel_count)
    {
        Vector3 world_pos(x, y, z);
        float distance = viewer.distance_to(world_pos);
        int lod = compute_lod(distance);
        Vector3i chunk_coord = Vector3i(x, y, z);
        int64_t key = chunk_hash(chunk_coord, lod);

        active_keys.insert(key);

        if (chunks.count(key) > 0)
            continue;

        ChunkNode* chunk = memnew(ChunkNode);
        chunk->configure(
            this,
            planet_data,
            chunk_coord,
            voxel_count,
            lod
        );
        chunk->set_position(world_pos);
        add_child(chunk);
        chunks[key] = chunk;
    }

    // Удаляем чанки, которых нет в активном наборе
    std::vector<int64_t> to_remove;

    for (auto& [key, chunk] : chunks) {
        if (active_keys.count(key) == 0) {
            to_remove.push_back(key);
        }
    }

    for (int64_t key : to_remove) {
        ChunkNode* chunk = chunks[key];
        chunk->queue_free();
        chunks.erase(key);
    }
}

ChunkNode* PlanetNode::get_chunk_by_origin(const Vector3i& origin) const {
    int64_t key = chunk_hash(origin, 1);
    auto it = chunks.find(key);
    if (it != chunks.end()) {
        return it->second;
    }
    return nullptr;
}