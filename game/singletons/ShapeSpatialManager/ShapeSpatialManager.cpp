#include "ShapeSpatialManager.hpp"
#include <WorldMesh/WorldMesh.hpp>

#include <godot_cpp/classes/engine.hpp>

#include <vector>

namespace godot {

ShapeSpatialManager* ShapeSpatialManager::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;

    Object* obj = engine->get_singleton("ShapeSpatialManager");
    return Object::cast_to<ShapeSpatialManager>(obj);
}

uint64_t ShapeSpatialManager::create_octree(
    uint64_t shape_id,
    const Vector3& position,
    float root_size,
    int max_depth
) {
    // Перерегистрация: если у фигуры уже было дерево, отвязываем её от него.
    detach_shape(shape_id);

    uint64_t octree_id = _next_octree_id++;

    ChunkOctree* octree = memnew(ChunkOctree);
    octree->id = octree_id;
    octree->set_position(position);
    octree->max_depth = max_depth;
    octree->add_user(shape_id);
    octree->add_root(Vector3(), root_size);

    if (WorldMesh* world_mesh = WorldMesh::get_singleton()) {
        world_mesh->add_child(octree);
    }

    _octrees.emplace(octree_id, octree);
    _shape_octree[shape_id] = octree_id;

    return octree_id;
}

uint64_t ShapeSpatialManager::rebuild_octree(
    uint64_t shape_id,
    float root_size,
    int max_depth
) {
    auto shape_it = _shape_octree.find(shape_id);
    if (shape_it == _shape_octree.end()) {
        return create_octree(shape_id, Vector3(), root_size, max_depth);
    }

    const uint64_t old_octree_id = shape_it->second;
    auto octree_it = _octrees.find(old_octree_id);
    if (octree_it == _octrees.end()) {
        return create_octree(shape_id, Vector3(), root_size, max_depth);
    }

    const Vector3 position = octree_it->second->get_position();

    std::vector<uint64_t> users;
    octree_it->second->for_each_user([&users](uint64_t user_id) {
        users.push_back(user_id);
    });

    destroy_octree(old_octree_id);

    const uint64_t new_octree_id = create_octree(shape_id, position, root_size, max_depth);

    for (uint64_t user_id : users) {
        if (user_id != shape_id) {
            attach_shape(user_id, new_octree_id);
        }
    }

    return new_octree_id;
}

void ShapeSpatialManager::set_octree_position(uint64_t shape_id, const Vector3& position) {
    auto shape_it = _shape_octree.find(shape_id);
    if (shape_it == _shape_octree.end()) {
        return;
    }

    auto octree_it = _octrees.find(shape_it->second);
    if (octree_it == _octrees.end()) {
        return;
    }

    octree_it->second->set_position(position);
}

void ShapeSpatialManager::attach_shape(uint64_t shape_id, uint64_t octree_id) {
    auto current_it = _shape_octree.find(shape_id);
    if (current_it != _shape_octree.end() && current_it->second == octree_id) {
        return;
    }

    detach_shape(shape_id);

    auto octree_it = _octrees.find(octree_id);
    if (octree_it == _octrees.end()) {
        return;
    }

    _shape_octree[shape_id] = octree_id;
    octree_it->second->add_user(shape_id);
}

void ShapeSpatialManager::detach_shape(uint64_t shape_id) {
    auto shape_it = _shape_octree.find(shape_id);
    if (shape_it == _shape_octree.end()) {
        return;
    }

    uint64_t octree_id = shape_it->second;
    _shape_octree.erase(shape_it);

    auto octree_it = _octrees.find(octree_id);
    if (octree_it == _octrees.end()) {
        return;
    }

    ChunkOctree* octree = octree_it->second;
    octree->remove_user(shape_id);

    if (octree->user_count() == 0) {
        destroy_octree(octree_id);
    }
}

void ShapeSpatialManager::destroy_octree(uint64_t octree_id) {
    auto octree_it = _octrees.find(octree_id);
    if (octree_it == _octrees.end()) {
        return;
    }

    ChunkOctree* octree = octree_it->second;

    std::vector<uint64_t> users;
    octree->for_each_user([&users](uint64_t user_id) {
        users.push_back(user_id);
    });

    for (uint64_t user_id : users) {
        _shape_octree.erase(user_id);
    }

    _octrees.erase(octree_it);

    octree->clear();
    octree->set_process(false);
    octree->queue_free();
}

void ShapeSpatialManager::bind_spatially(uint64_t child_shape_id, uint64_t parent_shape_id) {
    auto parent_it = _shape_octree.find(parent_shape_id);
    if (parent_it == _shape_octree.end()) {
        return;
    }

    uint64_t parent_octree_id = parent_it->second;
    if (parent_octree_id == 0) {
        return;
    }

    attach_shape(child_shape_id, parent_octree_id);
}

int ShapeSpatialManager::get_octree_max_depth(uint64_t shape_id) const {
    auto shape_it = _shape_octree.find(shape_id);
    if (shape_it == _shape_octree.end()) {
        return -1;
    }

    auto octree_it = _octrees.find(shape_it->second);
    if (octree_it == _octrees.end()) {
        return -1;
    }

    return octree_it->second->max_depth;
}

void ShapeSpatialManager::unregister_shape(uint64_t shape_id) {
    detach_shape(shape_id);
}

ChunkOctree* ShapeSpatialManager::get_octree(uint64_t shape_id) const {
    auto shape_it = _shape_octree.find(shape_id);
    if (shape_it == _shape_octree.end()) {
        return nullptr;
    }

    auto octree_it = _octrees.find(shape_it->second);
    if (octree_it == _octrees.end()) {
        return nullptr;
    }

    return octree_it->second;
}

Chunk* ShapeSpatialManager::get_chunk(uint64_t shape_id, uint64_t chunk_id) const {
    ChunkOctree* octree = get_octree(shape_id);
    if (!octree) {
        return nullptr;
    }

    return octree->find(chunk_id);
}

} // namespace godot