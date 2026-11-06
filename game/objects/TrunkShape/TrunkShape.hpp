#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <TrunkMeshGenerator/TrunkMeshGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Ствол дерева: низкополигональный конус с боковым изгибом.
class TrunkShape : public SemanticShape {
    GDCLASS(TrunkShape, SemanticShape)

private:
    Ref<PlanetShape> _planet;

    Vector3 _origin;
    Vector3 _up = Vector3(0.0f, 1.0f, 0.0f);
    float _height = 8.0f;
    float _base_radius = 0.5f;
    float _top_radius = 0.18f;
    Vector3 _bend;
    int _radial_segments = 7;
    int _height_segments = 6;
    float _ring_variation = 0.15f;

    Ref<TrunkMeshGenerator> _mesh_generator;
    Ref<TriplanarMaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

public:
    TrunkShape();
    virtual ~TrunkShape() = default;

    void bind_planet(const Ref<PlanetShape>& planet);

    void set_origin(const Vector3& v) { _origin = v; notify_shape_changed(); }
    Vector3 get_origin() const { return _origin; }

    void set_up(const Vector3& v);
    Vector3 get_up() const { return _up; }

    void set_height(float v) { _height = v; notify_shape_changed(); }
    float get_height() const { return _height; }

    void set_base_radius(float v) { _base_radius = v; notify_shape_changed(); }
    float get_base_radius() const { return _base_radius; }

    void set_top_radius(float v) { _top_radius = v; notify_shape_changed(); }
    float get_top_radius() const { return _top_radius; }

    void set_bend(const Vector3& v) { _bend = v; notify_shape_changed(); }
    Vector3 get_bend() const { return _bend; }

    void set_radial_segments(int v) { _radial_segments = v; notify_shape_changed(); }
    int get_radial_segments() const { return _radial_segments; }

    void set_height_segments(int v) { _height_segments = v; notify_shape_changed(); }
    int get_height_segments() const { return _height_segments; }

    void set_ring_variation(float v) { _ring_variation = v; notify_shape_changed(); }
    float get_ring_variation() const { return _ring_variation; }

    Vector3 centerline(float t) const;          // точка центральной линии, t in [0,1]
    Vector3 centerline_tangent(float t) const;  // нормализованная касательная
    float radius_at(float t) const;             // радиус конуса

    StringName get_shape_type() const override { return "trunk"; }
    void recompute_aabb() override;
    void on_register() override;
    void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot
