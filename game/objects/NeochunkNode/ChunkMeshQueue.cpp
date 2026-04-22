#include "ChunkMeshQueue.hpp"
#include "NeochunkNode.hpp"

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

        NeochunkNode* chunk = Object::cast_to<NeochunkNode>(
            ObjectDB::get_instance(job.chunk_id)
        );
        if (chunk) chunk->set_mesh(job.mesh);

        ++processed;
    }
}