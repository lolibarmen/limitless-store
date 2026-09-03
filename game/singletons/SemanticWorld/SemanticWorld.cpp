#include "SemanticWorld.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SemanticWorld::_bind_methods() {
    ClassDB::bind_method(D_METHOD("register_shape", "shape"), &SemanticWorld::register_shape);
    ClassDB::bind_method(D_METHOD("unregister_shape", "id"), &SemanticWorld::unregister_shape);
    ClassDB::bind_method(D_METHOD("get_shape", "id"), &SemanticWorld::get_shape);
    
    ClassDB::bind_method(D_METHOD("get_shape_count"), &SemanticWorld::get_shape_count);
    ClassDB::bind_method(D_METHOD("get_all_shapes"), &SemanticWorld::get_all_shapes);
    ClassDB::bind_method(D_METHOD("get_shapes_by_type", "type"), &SemanticWorld::get_shapes_by_type);
    
    ClassDB::bind_method(D_METHOD("mark_shape_dirty", "id"), &SemanticWorld::mark_shape_dirty);
    ClassDB::bind_method(D_METHOD("clear_dirty_flags"), &SemanticWorld::clear_dirty_flags);
}

uint64_t SemanticWorld::register_shape(Ref<SemanticShape> shape) {
    if (shape.is_null()) return 0;
    
    uint64_t id = _next_id++;
    shape->set_id(id);
    shape->set_world(this); // устанавливаем обратную связь
    _shapes[id] = shape;
    
    return id;
}

void SemanticWorld::unregister_shape(uint64_t id) {
    auto it = _shapes.find(id);
    if (it != _shapes.end()) {
        it->second->set_world(nullptr); // очищаем обратную связь
        _shapes.erase(it);
    }
}

Ref<SemanticShape> SemanticWorld::get_shape(uint64_t id) const {
    auto it = _shapes.find(id);
    return (it != _shapes.end()) ? it->second : Ref<SemanticShape>();
}

TypedArray<SemanticShape> SemanticWorld::get_all_shapes() const {
    TypedArray<SemanticShape> result;
    for (const auto& [id, shape] : _shapes) {
        result.push_back(shape);
    }
    return result;
}

TypedArray<SemanticShape> SemanticWorld::get_shapes_by_type(const String& type) const {
    TypedArray<SemanticShape> result;
    for (const auto& [id, shape] : _shapes) {
        if (shape->get_shape_type() == type) {
            result.push_back(shape);
        }
    }
    return result;
}

void SemanticWorld::mark_shape_dirty(uint64_t id) {
    _dirty_shapes.push_back(id);
    
    // Вызываем callback, если установлен
    if (_on_shape_changed) {
        auto shape = get_shape(id);
        if (shape.is_valid()) {
            _on_shape_changed(id, shape->get_aabb());
        }
    }
}

void SemanticWorld::clear_dirty_flags() {
    _dirty_shapes.clear();
}

void SemanticWorld::set_on_shape_changed(std::function<void(uint64_t, const AABB&)> callback) {
    _on_shape_changed = callback;
}