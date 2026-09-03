#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

namespace godot {

// Структура точки кривой (Hermite spline)
struct CurvePoint {
    Vector3 position;
    Vector3 tangent_in;   // Входящая касательная
    Vector3 tangent_out;  // Исходящаяا касательная
    float radius = 1.0f;
};

class SemanticCurve : public SemanticShape {
    GDCLASS(SemanticCurve, SemanticShape)

private:
    std::vector<CurvePoint> _points;
    bool _closed = false;

protected:
    static void _bind_methods();

public:
    SemanticCurve() = default;
    virtual ~SemanticCurve() = default;

    // --- Переопределения SemanticShape ---
    String get_shape_type() const override { return "curve"; }
    void recompute_aabb() override;

    // --- Управление точками ---
    void clear_points();
    void add_point(const Vector3& position, const Vector3& tangent_in = Vector3(), 
                   const Vector3& tangent_out = Vector3(), float radius = 1.0f);
    
    void set_point_position(int index, const Vector3& position);
    void set_point_radius(int index, float radius);
    
    int get_point_count() const { return (int)_points.size(); }
    Vector3 get_point_position(int index) const;
    float get_point_radius(int index) const;

    // --- Замкнутость ---
    void set_closed(bool closed);
    bool is_closed() const { return _closed; }

    // --- Семплирование (для генерации меша) ---
    // t - параметр от 0.0 до (get_point_count() - 1). 
    // Например, t=1.5 означает середину между 1-й и 2-й точкой.
    Vector3 sample(float t) const;
    Vector3 sample_tangent(float t) const;
    float sample_radius(float t) const;
};

} // namespace godot