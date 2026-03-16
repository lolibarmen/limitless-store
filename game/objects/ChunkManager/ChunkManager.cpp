#include "ChunkManager.hpp"
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
using namespace godot;

void ChunkManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_planet_data"), &ChunkManager::get_planet_data);
    ClassDB::bind_method(D_METHOD("set_planet_data", "p_data"), &ChunkManager::set_planet_data);
    ClassDB::add_property("ChunkManager",
        PropertyInfo(Variant::OBJECT, "planet_data", PROPERTY_HINT_RESOURCE_TYPE, "PlanetData"),
        "set_planet_data", "get_planet_data");
    ClassDB::bind_method(D_METHOD("update"), &ChunkManager::update);
}

ChunkManager::ChunkManager() {}

void ChunkManager::_ready() {
    planet_data.instantiate();
    block_generator.instantiate();
    update();
}

int ChunkManager::compute_lod(float distance) {
    if (distance < 64)  return 1;
    if (distance < 128) return 2;
    if (distance < 256) return 4;
    if (distance < 512) return 8;
    return 16;
}

void ChunkManager::_process(double delta) {
    update();
}

int64_t ChunkManager::chunk_hash(Vector3i pos, int lod) {
    int64_t h = 1469598103934665603ULL;
    h ^= pos.x; h *= 1099511628211ULL;
    h ^= pos.y; h *= 1099511628211ULL;
    h ^= pos.z; h *= 1099511628211ULL;
    h ^= lod;   h *= 1099511628211ULL;
    return h;
}

// Заполняет PlanetData данными от BlockGenerator для области чанка
void ChunkManager::populate_chunk_data(Vector3i chunk_origin, int voxel_count, int lod) {
    for (int x = 0; x < voxel_count; x++)
    for (int y = 0; y < voxel_count; y++)
    for (int z = 0; z < voxel_count; z++) {
        Vector3i world_pos = chunk_origin + Vector3i(x * lod, y * lod, z * lod);

        // BlockGenerator возвращает материал — переводим в плотность:
        // AIR = 0.0, всё остальное = 1.0 (можно расширить по необходимости)
        BlockMaterial mat = block_generator->get_block(world_pos);
        float density = (mat == BlockMaterial::AIR) ? -1.0f : 1.0f;

        planet_data->set_block(world_pos, lod, density);
    }
}

void ChunkManager::update() {
    Vector3 viewer = get_viewport()->get_camera_3d()->get_global_position();
    int range       = 32;
    int voxel_count = 8;

    // Округляем позицию камеры до сетки чанков
    Vector3i viewer_chunk(
        (int)Math::floor(viewer.x / voxel_count) * voxel_count,
        (int)Math::floor(viewer.y / voxel_count) * voxel_count,
        (int)Math::floor(viewer.z / voxel_count) * voxel_count
    );

    std::unordered_set<int64_t> active_keys;

    struct ChunkDesc {
        Vector3i coord;
        Vector3  world_pos;
        int      lod;
        int64_t  key;
    };
    std::vector<ChunkDesc> to_create;

    for (int x = -range; x < range; x += voxel_count)
    for (int y = -range; y < range; y += voxel_count)
    for (int z = -range; z < range; z += voxel_count) {
        Vector3i coord = viewer_chunk + Vector3i(x, y, z);
        Vector3  world_pos(coord.x, coord.y, coord.z);
        float    distance = viewer.distance_to(world_pos);
        int      lod      = compute_lod(distance);
        int64_t  key      = chunk_hash(coord, lod);

        active_keys.insert(key);

        if (chunks.count(key) == 0)
            to_create.push_back({ coord, world_pos, lod, key });
    }

    for (auto& desc : to_create)
        populate_chunk_data(desc.coord, voxel_count, desc.lod);

    for (auto& desc : to_create) {
        ChunkNode* chunk = memnew(ChunkNode);
        chunk->configure(this, planet_data, desc.coord, voxel_count, desc.lod);
        chunk->set_position(desc.world_pos);
        add_child(chunk);
        chunks[desc.key] = chunk;
    }

    std::vector<int64_t> to_remove;
    for (auto& [key, chunk] : chunks)
        if (active_keys.count(key) == 0)
            to_remove.push_back(key);

    for (int64_t key : to_remove) {
        chunks[key]->queue_free();
        chunks.erase(key);
    }
}

ChunkNode* ChunkManager::get_chunk_by_origin(const Vector3i& origin) const {
    int64_t key = chunk_hash(origin, 1);
    auto it = chunks.find(key);
    if (it != chunks.end())
        return it->second;
    return nullptr;
}