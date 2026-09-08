#include "ChunkMeshQueue.hpp"
#include "ChunkMeshNode.hpp"

using namespace godot;

void ChunkMeshQueue::tick(int max_per_frame) {
    int processed = 0;
    while (processed < max_per_frame) {
        ChunkMeshJob job;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_queue.empty()) break;
            job = _queue.front();
            _queue.pop();
        }

        ChunkMeshNode* chunk = Object::cast_to<ChunkMeshNode>(
            ObjectDB::get_instance(job.chunk_id)
        );
        if (chunk) chunk->set_mesh(job.mesh);

        ++processed;
    }
}

void ChunkMeshQueue::cancel_jobs_for(uint64_t chunk_id) {
    std::lock_guard<std::mutex> lock(_mutex);
    // Простой способ: создать новую очередь, отфильтровав ненужные
    std::queue<ChunkMeshJob> new_queue;
    while (!_queue.empty()) {
        ChunkMeshJob job = _queue.front();
        _queue.pop();
        if (job.chunk_id != chunk_id) {
            new_queue.push(job);
        }
    }
    _queue = std::move(new_queue);
}