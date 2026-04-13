#include "BlockLODSource.hpp"
#include <stdio.h>

using namespace godot;

void BlockLODSource::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_generator"),         &BlockLODSource::get_generator);
    ClassDB::bind_method(D_METHOD("set_generator", "gen"),  &BlockLODSource::set_generator);
    ClassDB::add_property("BlockLODSource",
        PropertyInfo(Variant::OBJECT, "generator",
                     PROPERTY_HINT_RESOURCE_TYPE, "BlockGenerator"),
        "set_generator", "get_generator");
}

void BlockLODSource::init(Ref<BlockGenerator> p_generator, Ref<BlockLODSource> p_block_lod_source) {
    if (!p_generator.is_valid()) {
        print_error("void BlockLODSource::init() p_generator not valid");
    }
    if (!p_block_lod_source.is_valid()) {
        print_error("void BlockLODSource::init() p_block_lod_source not valid");
    }
    generator = p_generator;
    block_lod_source = p_block_lod_source;

    mutex.instantiate();
}

BlockData BlockLODSource::get_block(const Vector3i& world_pos) const {
    mutex->lock();
    auto it = edits.find(world_pos);
    if (it != edits.end()) {
        BlockData result = it->second;
        mutex->unlock();
        return result;
    }
    mutex->unlock();
    return generator->get_block(world_pos);
}

void BlockLODSource::set_block(const Vector3i& world_pos, BlockData data) {
    mutex->lock();
    edits[world_pos] = data;
    mutex->unlock();
}

void BlockLODSource::reset_block(const Vector3i& world_pos) {
    mutex->lock();
    edits.erase(world_pos);
    mutex->unlock();
}

bool BlockLODSource::has_edit(const Vector3i& world_pos) const {
    mutex->lock();
    bool result = edits.count(world_pos) > 0;
    mutex->unlock();
    return result;
}

void BlockLODSource::fill_chunk(
    std::vector<BlockData>& out,
    const Vector3i& chunk_coord,
    int stride,
    int step
) const {
    const int total = stride * stride * stride;
    out.resize(total);

    // ШАГ 1: заполняем через генератор — без блокировки,
    // генератор читаем-only и не трогает edits
    int idx = 0;
    for (int x = 0; x < stride; x++)
    for (int y = 0; y < stride; y++)
    for (int z = 0; z < stride; z++, idx++)
    {
        Vector3i world_pos = chunk_coord + Vector3i(
            (x - 2) * step,
            (y - 2) * step,
            (z - 2) * step
        );
        out[idx] = generator->get_block(world_pos);
    }

    // ШАГ 2: накатываем edits поверх — берём снапшот под локом
    // чтобы не держать мьютекс во время тяжёлого цикла выше
    std::vector<std::pair<Vector3i, BlockData>> edits_snapshot;
    {
        mutex->lock();
        if (!edits.empty()) {
            edits_snapshot.reserve(edits.size());
            for (const auto& [pos, data] : edits) {
                edits_snapshot.emplace_back(pos, data);
            }
        }
        mutex->unlock();
    }

    if (edits_snapshot.empty()) return;

    // AABB чанка для фильтрации
    const Vector3i min_coord = chunk_coord + Vector3i(-2, -2, -2) * step;
    const Vector3i max_coord = chunk_coord + Vector3i(stride - 3, stride - 3, stride - 3) * step;

    for (const auto& [world_pos, data] : edits_snapshot) {
        // Отбрасываем эдиты вне чанка
        if (world_pos.x < min_coord.x || world_pos.x > max_coord.x) continue;
        if (world_pos.y < min_coord.y || world_pos.y > max_coord.y) continue;
        if (world_pos.z < min_coord.z || world_pos.z > max_coord.z) continue;

        // Проверяем кратность step для текущего LOD
        Vector3i local = world_pos - chunk_coord;
        if (local.x % step != 0 || local.y % step != 0 || local.z % step != 0) continue;

        const int lx = local.x / step + 2;
        const int ly = local.y / step + 2;
        const int lz = local.z / step + 2;

        if (lx < 0 || lx >= stride) continue;
        if (ly < 0 || ly >= stride) continue;
        if (lz < 0 || lz >= stride) continue;

        out[lx * stride * stride + ly * stride + lz] = data;
    }
}