#include "ShapeSpatialManager.hpp"

#include <godot_cpp/classes/engine.hpp>

#include <vector>

namespace godot {

ShapeSpatialManager* ShapeSpatialManager::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;

    Object* obj = engine->get_singleton("ShapeSpatialManager");
    return Object::cast_to<ShapeSpatialManager>(obj);
}

uint64_t ShapeSpatialManager::create_octree(uint64_t shape_id, const AABB& bounds) {
    uint64_t octree_id = _next_octree_id++;

    auto octree = std::make_unique<ChunkOctree>();
    octree->id = octree_id;
    octree->add_user(shape_id);
    octree->add_root(bounds.get_center());

    _octrees.emplace(octree_id, std::move(octree));
    _shape_octree[shape_id] = octree_id;

    return octree_id;
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

    ChunkOctree* octree = octree_it->second.get();
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

    ChunkOctree* octree = octree_it->second.get();

    octree->for_each_user([this](uint64_t user_id) {
        _shape_octree.erase(user_id);
    });

    _octrees.erase(octree_it);
}

void ShapeSpatialManager::register_shape(uint64_t shape_id, Ref<SemanticShape> shape) {
    if (!shape.is_valid()) {
        return;
    }

    detach_shape(shape_id);

    if (shape->is_relative()) {
        create_octree(shape_id, shape->get_aabb());
    }
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

    return octree_it->second.get();
}

Chunk* ShapeSpatialManager::get_chunk(uint64_t shape_id, uint64_t chunk_id) const {
    ChunkOctree* octree = get_octree(shape_id);
    if (!octree) {
        return nullptr;
    }

    return octree->find(chunk_id);
}

void ShapeSpatialManager::for_each_octree(
    std::function<void(uint64_t octree_id, ChunkOctree* octree)> func
) {
    std::vector<uint64_t> ids;
    ids.reserve(_octrees.size());

    for (const auto& pair : _octrees) {
        ids.push_back(pair.first);
    }

    for (uint64_t octree_id : ids) {
        auto it = _octrees.find(octree_id);
        if (it != _octrees.end()) {
            func(octree_id, it->second.get());
        }
    }
}

} // namespace godot