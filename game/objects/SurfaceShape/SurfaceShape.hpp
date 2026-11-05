#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <PlanetMaterialGenerator/PlanetMaterialGenerator.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Семантическая фигура поверхности планеты.
/// Владеет SDF, шумом рельефа, распределением материалов и генераторами поверхности.
/// Параметры формы (центр, радиус, амплитуда) читает у родительской PlanetShape:
/// планета — источник истины для фигур внутри неё.
class SurfaceShape : public SemanticShape {
    GDCLASS(SurfaceShape, SemanticShape)

private:
    Ref<PlanetShape> _planet;

    Ref<SurfaceGenerator> _mesh_generator;
    Ref<PlanetMaterialGenerator> _material_generator;

    Ref<FastNoiseLite> _noise;
    Ref<FastNoiseLite> _shared_voronoi;

    void _sync_from_planet();

protected:
    static void _bind_methods();

public:
    SurfaceShape();
    virtual ~SurfaceShape() = default;

    void bind_planet(const Ref<PlanetShape>& planet);

    /// @brief Возвращает ID материала (0-7) в заданной точке на поверхности планеты.
    int get_material_id(const Vector3& world_pos) const;

    StringName get_shape_type() const override { return "surface"; }
    void notify_shape_changed() override;
    float evaluate_sdf(const Vector3& world_pos) const override;
    void paint_vertices(const Vector3* positions, const Vector3* normals, Color* out_colors, int count, const Vector3& posoff) const override;
    void recompute_aabb() override;
    void on_register() override;
    void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot
