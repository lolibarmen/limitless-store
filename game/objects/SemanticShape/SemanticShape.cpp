#include "SemanticShape.hpp"
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SemanticShape::_bind_methods() {
    // Никаких BIND_ENUM_CONSTANT! Только чистые методы с int.
    ClassDB::bind_method(D_METHOD("set_property", "prop_id", "value"), &SemanticShape::set_property);
    ClassDB::bind_method(D_METHOD("get_property", "prop_id", "default_value"), &SemanticShape::get_property, DEFVAL(0.0f));
    ClassDB::bind_method(D_METHOD("has_property", "prop_id"), &SemanticShape::has_property);
    ClassDB::bind_method(D_METHOD("clear_property", "prop_id"), &SemanticShape::clear_property);
    
    ClassDB::bind_method(D_METHOD("set_custom", "key", "value"), &SemanticShape::set_custom);
    ClassDB::bind_method(D_METHOD("get_custom", "key", "default_value"), &SemanticShape::get_custom, DEFVAL(Variant()));
    ClassDB::bind_method(D_METHOD("get_all_custom"), &SemanticShape::get_all_custom);
    
    ClassDB::bind_method(D_METHOD("get_id"), &SemanticShape::get_id);
    ClassDB::bind_method(D_METHOD("get_shape_type"), &SemanticShape::get_shape_type);
    ClassDB::bind_method(D_METHOD("get_aabb"), &SemanticShape::get_aabb);
}

void SemanticShape::set_property(int prop_id, float value) {
    _properties[prop_id] = value;
    notify_shape_changed();
}

float SemanticShape::get_property(int prop_id, float default_value) const {
    auto it = _properties.find(prop_id);
    return (it != _properties.end()) ? it->second : default_value;
}

bool SemanticShape::has_property(int prop_id) const {
    return _properties.find(prop_id) != _properties.end();
}

void SemanticShape::clear_property(int prop_id) {
    _properties.erase(prop_id);
    notify_shape_changed();
}

void SemanticShape::set_custom(const String& key, const Variant& value) {
    _custom_properties[key] = value;
    notify_shape_changed();
}

Variant SemanticShape::get_custom(const String& key, const Variant& default_value) const {
    if (_custom_properties.has(key)) {
        return _custom_properties[key];
    }
    return default_value;
}

AABB SemanticShape::get_aabb() const {
    if (_aabb_dirty) {
        const_cast<SemanticShape*>(this)->recompute_aabb();
        const_cast<SemanticShape*>(this)->_aabb_dirty = false;
    }
    return _aabb;
}

void SemanticShape::notify_shape_changed() {
    _aabb_dirty = true;
    if (_world && _id != 0) {
        _world->mark_shape_dirty(_id);
    }
}