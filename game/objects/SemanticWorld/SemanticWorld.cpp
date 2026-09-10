#include "SemanticWorld.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

SemanticWorld::SemanticWorld() {
    _shapes_mutex.instantiate();
}

void SemanticWorld::_bind_methods() {
    ClassDB::bind_method(D_METHOD("register_shape", "shape"), &SemanticWorld::register_shape);
    ClassDB::bind_method(D_METHOD("unregister_shape", "id"), &SemanticWorld::unregister_shape);
    ClassDB::bind_method(D_METHOD("get_shape_count"), &SemanticWorld::get_shape_count);
    ClassDB::bind_method(D_METHOD("get_all_shapes"), &SemanticWorld::get_all_shapes);
    ClassDB::bind_method(D_METHOD("get_shapes_by_type", "type"), &SemanticWorld::get_shapes_by_type);
    ClassDB::bind_method(D_METHOD("mark_shape_dirty", "id"), &SemanticWorld::mark_shape_dirty);
    ClassDB::bind_method(D_METHOD("clear_dirty_flags"), &SemanticWorld::clear_dirty_flags);
}

uint64_t SemanticWorld::register_shape(Ref<SemanticShape> shape) {
    if (shape.is_null()) return 0;
    
    uint64_t id;
    _shapes_mutex->lock();
    id = _next_id++;
    shape->set_id(id);
    shape->set_world(this);
    _shapes[id] = shape;
    _shapes_mutex->unlock();
    
    return id;
}

void SemanticWorld::unregister_shape(uint64_t id) {
    _shapes_mutex->lock();
    auto it = _shapes.find(id);
    if (it != _shapes.end()) {
        it->second->set_world(nullptr);
        _shapes.erase(it);
    }
    _shapes_mutex->unlock();
}

std::vector<Ref<SemanticShape>> SemanticWorld::get_shapes_snapshot() const {
    std::vector<Ref<SemanticShape>> snapshot;
    _shapes_mutex->lock();
    snapshot.reserve(_shapes.size());
    for (const auto& pair : _shapes) {
        snapshot.push_back(pair.second);
    }
    _shapes_mutex->unlock();
    return snapshot;
}

int SemanticWorld::get_shape_count() const {
    _shapes_mutex->lock();
    int count = (int)_shapes.size();
    _shapes_mutex->unlock();
    return count;
}

TypedArray<SemanticShape> SemanticWorld::get_all_shapes() const {
    TypedArray<SemanticShape> result;
    _shapes_mutex->lock();
    for (const auto& pair : _shapes) {
        result.push_back(pair.second);
    }
    _shapes_mutex->unlock();
    return result;
}

TypedArray<SemanticShape> SemanticWorld::get_shapes_by_type(const String& type) const {
    TypedArray<SemanticShape> result;
    _shapes_mutex->lock();
    for (const auto& pair : _shapes) {
        if (pair.second->get_shape_type() == type) {
            result.push_back(pair.second);
        }
    }
    _shapes_mutex->unlock();
    return result;
}

void SemanticWorld::mark_shape_dirty(uint64_t id) {
    _shapes_mutex->lock();
    _dirty_shapes.push_back(id);
    _shapes_mutex->unlock();
}

void SemanticWorld::clear_dirty_flags() {
    _shapes_mutex->lock();
    _dirty_shapes.clear();
    _shapes_mutex->unlock();
}