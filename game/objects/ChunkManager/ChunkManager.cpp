#include "ChunkManager.hpp"
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>

using namespace godot;

void ChunkManager::_bind_methods() {
    // ClassDB::bind_method(D_METHOD("get_lod"),         &ChunkManager::get_lod);
    // ClassDB::bind_method(D_METHOD("set_lod", "v"),    &ChunkManager::set_lod);
    // ClassDB::add_property("ChunkManager",
    //     PropertyInfo(Variant::INT, "lod"),
    //     "set_lod", "get_lod");

    ClassDB::bind_method(D_METHOD("get_voxel_count"),      &ChunkManager::get_voxel_count);
    ClassDB::bind_method(D_METHOD("set_voxel_count", "v"), &ChunkManager::set_voxel_count);
    ClassDB::add_property("ChunkManager",
        PropertyInfo(Variant::INT, "voxel_count"),
        "set_voxel_count", "get_voxel_count");

    // ClassDB::bind_method(D_METHOD("get_range"),      &ChunkManager::get_range);
    // ClassDB::bind_method(D_METHOD("set_range", "v"), &ChunkManager::set_range);
    // ClassDB::add_property("ChunkManager",
    //     PropertyInfo(Variant::FLOAT, "range"),
    //     "set_range", "get_range");

    ClassDB::bind_method(D_METHOD("update"), &ChunkManager::update);
}

ChunkManager::ChunkManager() {}

void ChunkManager::_ready() {
    // update() не вызываем — ждём init() от WorldCoordinator
}

void ChunkManager::init(
    Ref<BlockLODSource> p_lod1,
    Ref<BlockLODSource> p_lod2,
    Ref<BlockLODSource> p_lod3
) {
    rings[0] = { 1, 0.0f,       range_lod1, p_lod1, {} };
    rings[1] = { 2, range_lod1, range_lod2, p_lod2, {} };
    rings[2] = { 4, range_lod2, range_lod3, p_lod3, {} };

    // update();
}

static int iter_cococo = 0;

void ChunkManager::_process(double delta) {
    update();
}

ChunkNode* ChunkManager::get_chunk_by_origin(const Vector3i& origin) const {
    int64_t key = chunk_hash(origin);
    for (const auto& ring : rings) {
        auto it = ring.chunks.find(key);
        if (it != ring.chunks.end())
            return it->second;
    }
    return nullptr;
}

int64_t ChunkManager::chunk_hash(Vector3i pos) {
    int64_t h = 1469598103934665603ULL;
    h ^= (int64_t)pos.x; h *= 1099511628211ULL;
    h ^= (int64_t)pos.y; h *= 1099511628211ULL;
    h ^= (int64_t)pos.z; h *= 1099511628211ULL;
    return h;
}

// Снапнуть координату на сетку с шагом step (floor-snap)
static Vector3i snap(Vector3i v, int step) {
    auto fs = [&](int x) -> int {
        return (x >= 0) ? (x / step * step)
                        : ((x - step + 1) / step * step);
    };
    return { fs(v.x), fs(v.y), fs(v.z) };
}

void ChunkManager::spawn_chunk(Ring& ring, const Vector3i& coord)
{
    int64_t key = chunk_hash(coord);
    if (ring.chunks.count(key)) return;

    ChunkNode* chunk = memnew(ChunkNode);
    chunk->init(this, ring.source, coord, voxel_count, ring.lod);
    chunk->set_position(Vector3(coord.x, coord.y, coord.z));
    add_child(chunk);
    ring.chunks[key] = chunk;
}

void ChunkManager::despawn_chunk(Ring& ring, int64_t key)
{
    auto it = ring.chunks.find(key);
    if (it == ring.chunks.end()) return;
    it->second->queue_free();
    ring.chunks.erase(it);
}

void ChunkManager::update()
{
    auto* vp = get_viewport();
    if (!vp) return;
    auto* cam = vp->get_camera_3d();
    if (!cam) return;

    Vector3 viewer_pos = cam->get_global_position();

    // Итерируем по сетке самого крупного кольца
    const Ring& outer   = rings[RINGS_COUNT - 1];
    int   coarse_size   = voxel_count * outer.lod;
    float cs            = (float)coarse_size;
    int   irange        = (int)Math::ceil(outer.r_outer / cs);

    Vector3i viewer_snapped = snap(
        Vector3i((int)viewer_pos.x, (int)viewer_pos.y, (int)viewer_pos.z),
        coarse_size
    );

    // Собираем desired для каждого кольца
    std::unordered_set<int64_t> desired[RINGS_COUNT];

    for (int x = -irange; x <= irange; ++x)
    for (int y = -irange; y <= irange; ++y)
    for (int z = -irange; z <= irange; ++z)
    {
        Vector3i coarse_coord = viewer_snapped + Vector3i(x, y, z) * coarse_size;
        Vector3  coarse_center(
            coarse_coord.x + cs * 0.5f,
            coarse_coord.y + cs * 0.5f,
            coarse_coord.z + cs * 0.5f
        );
        float dist = viewer_pos.distance_to(coarse_center);

        if (dist > outer.r_outer) continue;

        // Находим нужное кольцо по дистанции
        int ring_idx = RINGS_COUNT - 1;
        for (int i = 0; i < RINGS_COUNT; ++i) {
            if (dist <= rings[i].r_outer) {
                ring_idx = i;
                break;
            }
        }

        Ring& ring      = rings[ring_idx];
        int   fine_size = voxel_count * ring.lod;
        int   subdiv    = coarse_size / fine_size; // сколько мелких чанков в одном крупном

        // Subdivide крупную ячейку на чанки нужного LOD
        for (int sx = 0; sx < subdiv; ++sx)
        for (int sy = 0; sy < subdiv; ++sy)
        for (int sz = 0; sz < subdiv; ++sz)
        {
            Vector3i coord = coarse_coord + Vector3i(sx, sy, sz) * fine_size;
            int64_t  key   = chunk_hash(coord);
            desired[ring_idx].insert(key);

            if (ring.chunks.count(key) == 0)
                spawn_chunk(ring, coord);
        }
    }

    // Despawn по каждому кольцу
    for (int i = 0; i < RINGS_COUNT; ++i) {
        std::vector<int64_t> to_remove;
        for (auto& [key, _] : rings[i].chunks)
            if (!desired[i].count(key))
                to_remove.push_back(key);
        for (int64_t key : to_remove)
            despawn_chunk(rings[i], key);
    }
}