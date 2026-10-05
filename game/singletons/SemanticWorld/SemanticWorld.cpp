#include "SemanticWorld.hpp"
#include <ChunkOctree/ChunkOctree.hpp> // Добавлено для доступа к Chunk
#include <ChunkOctree/Chunk.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

SemanticWorld::SemanticWorld() {
    _shapes_mutex.instantiate();
}

void SemanticWorld::_bind_methods() {
    // ... ваши существующие привязки методов
}

SemanticWorld* SemanticWorld::get_singleton() {
    Engine* engine = Engine::get_singleton();
    if (!engine) return nullptr;
    Object* obj = engine->get_singleton("SemanticWorld");
    return Object::cast_to<SemanticWorld>(obj);
}

uint64_t SemanticWorld::register_shape(Ref<SemanticShape> shape) {
    if (!shape.is_valid()) return 0; 
    uint64_t id = _next_id++;
    shape->set_id(id);
    _shapes_mutex->lock();
    _shapes[id] = shape;
    _shapes_mutex->unlock();
    return id;
}

void SemanticWorld::unregister_shape(uint64_t id) {
    _shapes_mutex->lock();
    _shapes.erase(id);
    _shapes_mutex->unlock();
}

Ref<SemanticShape> SemanticWorld::get_shape(uint64_t id) const {
    _shapes_mutex->lock();
    auto it = _shapes.find(id);
    Ref<SemanticShape> result = (it != _shapes.end()) ? it->second : nullptr;
    _shapes_mutex->unlock();
    return result;
}

std::vector<Ref<SemanticShape>> SemanticWorld::get_shapes_snapshot() const {
    std::vector<Ref<SemanticShape>> snapshot;
    _shapes_mutex->lock();
    snapshot.reserve(_shapes.size());
    for (const auto& pair : _shapes) {
        if (pair.second.is_valid()) {
            snapshot.push_back(pair.second);
        }
    }
    _shapes_mutex->unlock();
    return snapshot;
}

int SemanticWorld::get_shape_count() const {
    _shapes_mutex->lock();
    int count = _shapes.size();
    _shapes_mutex->unlock();
    return count;
}

TypedArray<SemanticShape> SemanticWorld::get_all_shapes() const {
    TypedArray<SemanticShape> arr;
    _shapes_mutex->lock();
    for (const auto& pair : _shapes) {
        if (pair.second.is_valid()) {
            arr.append(pair.second);
        }
    }
    _shapes_mutex->unlock();
    return arr;
}

TypedArray<SemanticShape> SemanticWorld::get_shapes_by_type(const String& type) const {
    TypedArray<SemanticShape> arr;
    _shapes_mutex->lock();
    for (const auto& pair : _shapes) {
        if (pair.second.is_valid() && pair.second->get_class() == type) {
            arr.append(pair.second);
        }
    }
    _shapes_mutex->unlock();
    return arr;
}

void SemanticWorld::on_zone_changed(const std::vector<uint64_t>& chunk_ids) {
    if (chunk_ids.empty()) return;

    auto* octree = ChunkOctree::get_singleton();

    for (uint64_t chunk_id : chunk_ids) {
        Chunk* chunk = octree ? octree->find(chunk_id) : nullptr;
        
        std::vector<uint64_t> affected_shape_ids;

        _shapes_mutex->lock();
        for (const auto& pair : _shapes) {
            if (pair.second.is_valid()) {
                if (chunk) {
                    if (pair.second->get_aabb().intersects(chunk->get_aabb())) {
                        affected_shape_ids.push_back(pair.first);
                    }
                } else {
                    affected_shape_ids.push_back(pair.first);
                }
            }
        }
        _shapes_mutex->unlock();

        for (uint64_t id : affected_shape_ids) {
            Ref<SemanticShape> shape = get_shape(id);
            if (shape.is_valid()) {
                shape->on_zone_changed(chunk_id);
            }
        }
    }
}