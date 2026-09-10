#include "SemanticSphere.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SemanticSphere::_bind_methods() {
    // Привязка геттеров и сеттеров для центра
    ClassDB::bind_method(D_METHOD("set_center", "center"), &SemanticSphere::set_center);
    ClassDB::bind_method(D_METHOD("get_center"), &SemanticSphere::get_center);
    
    // Привязка геттеров и сеттеров для радиуса
    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SemanticSphere::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &SemanticSphere::get_radius);

    // Добавление свойств в инспектор Godot (опционально, но очень рекомендуется)
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.01,1000.0,0.01"), "set_radius", "get_radius");
}

void SemanticSphere::recompute_aabb() {
    // AABB сферы: центр минус радиус (минимальная точка), размер = радиус * 2
    Vector3 extent(_radius, _radius, _radius);
    _aabb = AABB(_center - extent, extent * 2.0f);
    _aabb_dirty = false;
}

float SemanticSphere::evaluate_sdf(const Vector3& world_pos) const {
    // Классическая SDF для сферы: расстояние от точки до центра минус радиус.
    // Результат > 0: точка снаружи сферы
    // Результат < 0: точка внутри сферы
    // Результат == 0: точка на поверхности
    return world_pos.distance_to(_center) - _radius;
}

void SemanticSphere::set_center(const Vector3& center) {
    if (_center != center) {
        _center = center;
        notify_shape_changed();
    }
}

void SemanticSphere::set_radius(float radius) {
    // Защита от отрицательного или нулевого радиуса, который может сломать логику
    float clamped_radius = MAX(0.001f, radius);
    if (Math::abs(_radius - clamped_radius) > 0.001f) {
        _radius = clamped_radius;
        notify_shape_changed();
    }
}