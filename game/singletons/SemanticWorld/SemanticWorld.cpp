#include "SemanticWorld.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

SemanticWorld::SemanticWorld() {
    _shapes_mutex.instantiate(); // Инициализация Godot Mutex
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
    
    _shapes_mutex->lock();
    uint64_t id = _next_id++;
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

// КЛЮЧЕВОЙ МЕТОД: Делаем снапшот под локом, отпускаем лок, возвращаем результат
std::vector<Ref<SemanticShape>> SemanticWorld::get_shapes_snapshot() const {
    std::vector<Ref<SemanticShape>> snapshot;
    
    _shapes_mutex->lock();
    snapshot.reserve(_shapes.size());
    for (const auto& pair : _shapes) {
        snapshot.push_back(pair.second); // Копирование Ref безопасно увеличивает счетчик
    }
    _shapes_mutex->unlock(); // Лок отпущен ДО возврата из функции
    
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
    Ref<SemanticShape> shape_copy;
    AABB bounds;
    bool has_callback = false;
    
    // Минимально возможный лок: только чтобы достать данные и добавить в dirty
    _shapes_mutex->lock();
    _dirty_shapes.push_back(id);
    auto it = _shapes.find(id);
    if (it != _shapes.end()) {
        shape_copy = it->second;
        bounds = shape_copy->get_aabb();
    }
    has_callback = (_on_shape_changed != nullptr);
    _shapes_mutex->unlock(); // Сразу отпускаем!

    // Вызов callback вне лока, чтобы избежать deadlock, если callback 
    // попытается сделать что-то с SemanticWorld (например, запросить фигуры)
    if (has_callback && shape_copy.is_valid()) {
        _on_shape_changed(id, bounds);
    }
}

void SemanticWorld::clear_dirty_flags() {
    _shapes_mutex->lock();
    _dirty_shapes.clear();
    _shapes_mutex->unlock();
}

void SemanticWorld::set_on_shape_changed(std::function<void(uint64_t, const AABB&)> callback) {
    _shapes_mutex->lock();
    _on_shape_changed = callback;
    _shapes_mutex->unlock();
}