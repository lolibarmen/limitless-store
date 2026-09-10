#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

class SemanticSphere : public SemanticShape {
    GDCLASS(SemanticSphere, SemanticShape)

private:
    Vector3 _center;
    float _radius = 1.0f;

protected:
    static void _bind_methods();

public:
    SemanticSphere() = default;
    virtual ~SemanticSphere() = default;

    // --- Переопределения SemanticShape ---
    String get_shape_type() const override { return "sphere"; }
    void recompute_aabb() override;
    float evaluate_sdf(const Vector3& world_pos) const override;

    // --- Управление параметрами сферы ---
    void set_center(const Vector3& center);
    Vector3 get_center() const { return _center; }

    void set_radius(float radius);
    float get_radius() const { return _radius; }
};

} // namespace godot