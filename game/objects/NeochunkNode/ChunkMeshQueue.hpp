#pragma once
#include <godot_cpp/classes/mesh.hpp>
#include <mutex>
#include <queue>

namespace godot {

struct ChunkMeshJob {
    uint64_t chunk_id;
    Ref<Mesh> mesh;
};

class ChunkMeshQueue {
public:
    static ChunkMeshQueue& get_singleton() {
        static ChunkMeshQueue instance;
        return instance;
    }

    void push(uint64_t chunk_id, Ref<Mesh> mesh) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push({ chunk_id, mesh });
    }

    void tick(int max_per_frame = 4);

private:
    ChunkMeshQueue() = default;

    std::mutex _mutex;
    std::queue<ChunkMeshJob> _queue;
};

} // namespace godot