#include "ChunkManager.hpp"
#include <unordered_set>

using namespace godot;

void ChunkManager::initialize(Ref<PlanetData> p_data, Ref<BiomeGenerator> p_generator) {
    planet_data     = p_data;
    biome_generator = p_generator;
}

int64_t ChunkManager::chunk_hash(const Vector3i& pos, int lod) {
    int64_t h = 1469598103934665603ULL;

    h ^= pos.x; h *= 1099511628211ULL;
    h ^= pos.y; h *= 1099511628211ULL;
    h ^= pos.z; h *= 1099511628211ULL;
    h ^= lod;   h *= 1099511628211ULL;

    return h;
}

void ChunkManager::spawn_chunk(const Slot& slot) {
    Dictionary extra = slot.extra;
    Vector3i origin  = extra["origin"];
    int lod          = extra["lod"];

    int64_t key = chunk_hash(origin, lod);
    if (chunks.count(key) > 0) return;

    // Заполняем PlanetData через генератор если чанк новый
    int voxel_count = 8;
    for (int x = 0; x < voxel_count; ++x)
    for (int y = 0; y < voxel_count; ++y)
    for (int z = 0; z < voxel_count; ++z) {
        Vector3i vpos  = origin + Vector3i(x, y, z) * lod;
        Vector3  wpos  = Vector3(vpos.x, vpos.y, vpos.z);
        float density  = biome_generator->get_density(wpos);
        planet_data->set_block(vpos, lod, density);
    }

    ChunkNode* chunk = memnew(ChunkNode);
    chunk->configure(this, planet_data, origin, voxel_count, lod);
    chunk->set_position(slot.position);
    add_child(chunk);

    chunks[key] = chunk;
}

void ChunkManager::despawn_chunk(int64_t key) {
    auto it = chunks.find(key);
    if (it == chunks.end()) return;

    it->second->queue_free();
    chunks.erase(it);
}

void ChunkManager::update(const Vector3& camera_pos) {
    if (biome_generator.is_null() || planet_data.is_null()) return;

    // Запрашиваем у генератора актуальный набор слотов
    std::vector<Slot> wanted = biome_generator->query(camera_pos, 512.0f);

    // Собираем ключи нужных чанков
    std::unordered_set<int64_t> active_keys;
    for (const Slot& slot : wanted) {
        Dictionary extra = slot.extra;
        Vector3i origin  = extra["origin"];
        int lod          = extra["lod"];
        int64_t key      = chunk_hash(origin, lod);

        active_keys.insert(key);
        spawn_chunk(slot);
    }

    // Убираем чанки которых нет в активном наборе
    std::vector<int64_t> to_remove;
    for (auto& [key, chunk] : chunks) {
        if (active_keys.count(key) == 0)
            to_remove.push_back(key);
    }

    for (int64_t key : to_remove)
        despawn_chunk(key);
}

ChunkNode* ChunkManager::get_chunk_by_origin(const Vector3i& origin) const {
    int64_t key = chunk_hash(origin, 1);
    auto it = chunks.find(key);
    if (it != chunks.end()) return it->second;
    return nullptr;
}