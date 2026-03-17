#include "ChunkManager.hpp"
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>

using namespace godot;

void ChunkManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_lod"),         &ChunkManager::get_lod);
    ClassDB::bind_method(D_METHOD("set_lod", "v"),    &ChunkManager::set_lod);
    ClassDB::add_property("ChunkManager",
        PropertyInfo(Variant::INT, "lod"),
        "set_lod", "get_lod");

    ClassDB::bind_method(D_METHOD("get_voxel_count"),      &ChunkManager::get_voxel_count);
    ClassDB::bind_method(D_METHOD("set_voxel_count", "v"), &ChunkManager::set_voxel_count);
    ClassDB::add_property("ChunkManager",
        PropertyInfo(Variant::INT, "voxel_count"),
        "set_voxel_count", "get_voxel_count");

    ClassDB::bind_method(D_METHOD("get_range"),      &ChunkManager::get_range);
    ClassDB::bind_method(D_METHOD("set_range", "v"), &ChunkManager::set_range);
    ClassDB::add_property("ChunkManager",
        PropertyInfo(Variant::FLOAT, "range"),
        "set_range", "get_range");

    ClassDB::bind_method(D_METHOD("update"), &ChunkManager::update);
}

ChunkManager::ChunkManager() {}

void ChunkManager::_ready() {
    // update() не вызываем — ждём init() от WorldCoordinator
}

void ChunkManager::init(Ref<BlockSource> p_block_source) {
    ERR_FAIL_COND_MSG(!p_block_source.is_valid(),
        "ChunkManager::init: block_source невалиден");

    block_source = p_block_source;
    update();
}

void ChunkManager::_process(double delta) {
    // Не обновляем если источник ещё не передан
    if (!block_source.is_valid()) return;
    update();
}

int64_t ChunkManager::chunk_hash(Vector3i pos) {
    int64_t h = 1469598103934665603ULL;
    h ^= (int64_t)pos.x; h *= 1099511628211ULL;
    h ^= (int64_t)pos.y; h *= 1099511628211ULL;
    h ^= (int64_t)pos.z; h *= 1099511628211ULL;
    return h;
}

void ChunkManager::update() {
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    Vector3 viewer     = cam->get_global_position();
    float   chunk_step = (float)(voxel_count * lod);

    Vector3i viewer_chunk(
        (int)Math::floor(viewer.x / chunk_step) * (int)chunk_step,
        (int)Math::floor(viewer.y / chunk_step) * (int)chunk_step,
        (int)Math::floor(viewer.z / chunk_step) * (int)chunk_step
    );

    std::unordered_set<int64_t> active_keys;
    std::vector<Vector3i>       to_create;

    int irange = (int)Math::ceil(range / chunk_step);

    for (int x = -irange; x < irange; x++)
    for (int y = -irange; y < irange; y++)
    for (int z = -irange; z < irange; z++) {
        Vector3i coord = viewer_chunk + Vector3i(
            x * (int)chunk_step,
            y * (int)chunk_step,
            z * (int)chunk_step
        );

        Vector3 world_pos(coord.x, coord.y, coord.z);
        if (viewer.distance_to(world_pos) > range)
            continue;

        int64_t key = chunk_hash(coord);
        active_keys.insert(key);

        if (chunks.count(key) == 0)
            to_create.push_back(coord);
    }

    for (const Vector3i& coord : to_create) {
        int64_t    key   = chunk_hash(coord);
        ChunkNode* chunk = memnew(ChunkNode);
        chunk->init(this, block_source, coord, voxel_count, lod);
        chunk->set_position(Vector3(coord.x, coord.y, coord.z));
        add_child(chunk);
        chunks[key] = chunk;
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
    int64_t key = chunk_hash(origin);
    auto it = chunks.find(key);
    if (it != chunks.end())
        return it->second;
    return nullptr;
}