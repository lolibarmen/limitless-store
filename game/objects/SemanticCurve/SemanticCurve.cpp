#include "SemanticCurve.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SemanticCurve::_bind_methods() {
    // Управление точками
    ClassDB::bind_method(D_METHOD("clear_points"), &SemanticCurve::clear_points);
    ClassDB::bind_method(D_METHOD("add_point", "position", "tangent_in", "tangent_out", "radius"), 
                         &SemanticCurve::add_point, DEFVAL(Vector3()), DEFVAL(Vector3()), DEFVAL(1.0f));
    
    ClassDB::bind_method(D_METHOD("set_point_position", "index", "position"), &SemanticCurve::set_point_position);
    ClassDB::bind_method(D_METHOD("set_point_radius", "index", "radius"), &SemanticCurve::set_point_radius);
    
    ClassDB::bind_method(D_METHOD("get_point_count"), &SemanticCurve::get_point_count);
    ClassDB::bind_method(D_METHOD("get_point_position", "index"), &SemanticCurve::get_point_position);
    ClassDB::bind_method(D_METHOD("get_point_radius", "index"), &SemanticCurve::get_point_radius);
    
    // Замкнутость
    ClassDB::bind_method(D_METHOD("set_closed", "closed"), &SemanticCurve::set_closed);
    ClassDB::bind_method(D_METHOD("is_closed"), &SemanticCurve::is_closed);
    
    // Семплирование
    ClassDB::bind_method(D_METHOD("sample", "t"), &SemanticCurve::sample);
    ClassDB::bind_method(D_METHOD("sample_tangent", "t"), &SemanticCurve::sample_tangent);
    ClassDB::bind_method(D_METHOD("sample_radius", "t"), &SemanticCurve::sample_radius);
}

void SemanticCurve::recompute_aabb() {
    if (_points.empty()) {
        _aabb = AABB();
        return;
    }
    
    Vector3 min_pt = _points[0].position;
    Vector3 max_pt = _points[0].position;
    float max_radius = _points[0].radius;
    
    for (const auto& p : _points) {
        min_pt = min_pt.min(p.position);
        max_pt = max_pt.max(p.position);
        max_radius = MAX(max_radius, p.radius);
    }
    
    // Расширяем AABB на максимальный радиус, чтобы кривая гарантированно в него влезала
    Vector3 expansion(max_radius, max_radius, max_radius);
    _aabb = AABB(min_pt - expansion, (max_pt - min_pt) + expansion * 2.0f);
}

void SemanticCurve::clear_points() {
    _points.clear();
    notify_shape_changed();
}

void SemanticCurve::add_point(const Vector3& position, const Vector3& tangent_in,
                              const Vector3& tangent_out, float radius) {
    CurvePoint p;
    p.position = position;
    p.tangent_in = tangent_in;
    p.tangent_out = tangent_out;
    p.radius = radius;
    _points.push_back(p);
    notify_shape_changed();
}

void SemanticCurve::set_point_position(int index, const Vector3& position) {
    if (index >= 0 && index < (int)_points.size()) {
        _points[index].position = position;
        notify_shape_changed();
    }
}

void SemanticCurve::set_point_radius(int index, float radius) {
    if (index >= 0 && index < (int)_points.size()) {
        _points[index].radius = radius;
        notify_shape_changed();
    }
}

Vector3 SemanticCurve::get_point_position(int index) const {
    if (index >= 0 && index < (int)_points.size()) {
        return _points[index].position;
    }
    return Vector3();
}

float SemanticCurve::get_point_radius(int index) const {
    if (index >= 0 && index < (int)_points.size()) {
        return _points[index].radius;
    }
    return 0.0f;
}

void SemanticCurve::set_closed(bool closed) {
    _closed = closed;
    notify_shape_changed();
}

// --- Вспомогательная функция: Кубическая интерполяция Эрмита ---
static Vector3 hermite(const Vector3& p0, const Vector3& m0, 
                       const Vector3& p1, const Vector3& m1, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h01 = -2.0f * t3 + 3.0f * t2;
    float h11 = t3 - t2;
    return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
}

Vector3 SemanticCurve::sample(float t) const {
    int n = (int)_points.size();
    if (n < 2) return n == 1 ? _points[0].position : Vector3();
    
    int segment = (int)Math::floor(t);
    float local_t = t - segment;
    
    if (_closed) {
        segment = ((segment % n) + n) % n; // Циклический сдвиг
    } else {
        segment = CLAMP(segment, 0, n - 2); // Ограничиваем краями
    }
    
    int i0 = segment;
    int i1 = (segment + 1) % n;
    
    return hermite(_points[i0].position, _points[i0].tangent_out,
                   _points[i1].position, _points[i1].tangent_in, local_t);
}

Vector3 SemanticCurve::sample_tangent(float t) const {
    // Численное дифференцирование (достаточно точно для генерации меша)
    const float eps = 0.001f;
    return (sample(t + eps) - sample(t - eps)).normalized();
}

float SemanticCurve::sample_radius(float t) const {
    int n = (int)_points.size();
    if (n == 0) return 0.0f;
    if (n == 1) return _points[0].radius;
    
    int segment = (int)Math::floor(t);
    float local_t = t - segment;
    
    if (_closed) {
        segment = ((segment % n) + n) % n;
    } else {
        segment = CLAMP(segment, 0, n - 2);
    }
    
    int i0 = segment;
    int i1 = (segment + 1) % n;
    
    return Math::lerp(_points[i0].radius, _points[i1].radius, local_t);
}