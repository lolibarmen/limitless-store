#pragma once
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <unordered_map>
#include <vector>
#include <Utils/SpatialHash.hpp>

namespace godot {

// Универсальный интерфейс-хранилище данных чанкового мира.
// Хранит "эдиты" (изменения игрока/системы) поверх процедурного генератора.
// T — тип хранимых данных (BlockData, BiomeData, LightData и т.д.)
template<typename T>
class Source {
protected:
    std::unordered_map<Vector3i, T, Vector3iHash> edits;
    Ref<Mutex> mutex;

    // Точка расширения: получить "сырое" значение из генератора.
    // Наследник должен переопределить и дернуть свой конкретный генератор.
    virtual T generate(const Vector3i& world_pos) const = 0;

public:
    Source() { mutex.instantiate(); }
    virtual ~Source() = default;

    // Чтение: эдит, если есть, иначе генератор
    T get_value(const Vector3i& world_pos) const {
        mutex->lock();
        auto it = edits.find(world_pos);
        if (it != edits.end()) {
            T result = it->second;
            mutex->unlock();
            return result;
        }
        mutex->unlock();
        return generate(world_pos);
    }

    // Запись эдита
    void set_value(const Vector3i& world_pos, T data) {
        mutex->lock();
        edits[world_pos] = data;
        mutex->unlock();
    }

    // Сброс эдита (возврат к генератору)
    void reset_value(const Vector3i& world_pos) {
        mutex->lock();
        edits.erase(world_pos);
        mutex->unlock();
    }

    bool has_edit_value(const Vector3i& world_pos) const {
        mutex->lock();
        bool result = edits.count(world_pos) > 0;
        mutex->unlock();
        return result;
    }

    size_t edit_count() const {
        mutex->lock();
        size_t n = edits.size();
        mutex->unlock();
        return n;
    }

    // Заполняет массив данных для чанка: сначала генератором, затем эдиты поверх.
    // stride  — размер стороны буфера (с учётом краевого запаса, как в исходнике: -2..stride-3)
    // step    — шаг сетки для текущего LOD
    void fill_chunk(
        std::vector<T>& out,
        const Vector3i& chunk_coord,
        int stride,
        int step
    ) const {
        const int total = stride * stride * stride;
        out.resize(total);

        // ШАГ 1: генератор, без лока — генератор read-only и не трогает edits
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
            out[idx] = generate(world_pos);
        }

        // ШАГ 2: снапшот эдитов под локом, накатываем без лока
        std::vector<std::pair<Vector3i, T>> edits_snapshot;
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

        const Vector3i min_coord = chunk_coord + Vector3i(-2, -2, -2) * step;
        const Vector3i max_coord = chunk_coord + Vector3i(stride - 3, stride - 3, stride - 3) * step;

        for (const auto& [world_pos, data] : edits_snapshot) {
            if (world_pos.x < min_coord.x || world_pos.x > max_coord.x) continue;
            if (world_pos.y < min_coord.y || world_pos.y > max_coord.y) continue;
            if (world_pos.z < min_coord.z || world_pos.z > max_coord.z) continue;

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
};

} // namespace godot