#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Фигура, подобная MaterialTerrainShape, но с мешем в форме сферы.
/// Геометрия строится SurfaceGenerator по SDF: dist(p - center) - radius.
class SphericalTerrainShape : public SemanticShape {
    GDCLASS(SphericalTerrainShape, SemanticShape)

private:
    Vector3 _center;
    float _radius = 10.0f;

    Ref<SurfaceGenerator> _generator;
    Ref<TriplanarMaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

public:
    SphericalTerrainShape();
    virtual ~SphericalTerrainShape() = default;

    void set_center(const Vector3& c) { _center = c; notify_shape_changed(); }
    Vector3 get_center() const { return _center; }

    void set_radius(float r) { _radius = r; notify_shape_changed(); }
    float get_radius() const { return _radius; }

    StringName get_shape_type() const override { return "spherical_terrain"; }
    float evaluate_sdf(const Vector3& world_pos) const override;
    void recompute_aabb() override;
    void on_zone_changed(uint64_t chunk_id) override;

    /// @brief Фигуре нужно собственное октодерево — при регистрации она сама создаёт его
    /// в ShapeSpatialManager со своими параметрами (максимальная глубина, размер корня).
    void on_register() override;
};

} // namespace godot
