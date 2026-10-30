#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <PlanetTerrainShape/PlanetTerrainShape.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {
    
/// @brief Планета: набор PlanetTerrainShape с разными текстурами материалов
/// (земля, трава, гравий, песок, снег, камень, вода).
/// Создаёт общее октодерево и привязывает к нему все свои фигуры через bind_spatially().
/// Единственный источник SDF: базовый SDF ландшафта вычисляется здесь; каждая
/// PlanetTerrainShape берёт его через bind_material() и отсекает по своей ячейке Вороного.
class PlanetShape : public SemanticShape {
    GDCLASS(PlanetShape, SemanticShape)

private:
    Vector3 _center;
    float _radius = 100.0f;
    float _amplitude = 20.0f;

    /// Шум высот — часть базового SDF планеты (перенесён из PlanetTerrainShape::evaluate_sdf).
    Ref<FastNoiseLite> _noise;
    /// Общий воронои-шум: распределяет материалы PlanetTerrainShape по поверхности планеты.
    Ref<FastNoiseLite> _shared_voronoi;

    Ref<PlanetTerrainShape> _dirt_shape;
    Ref<PlanetTerrainShape> _grass_shape;
    Ref<PlanetTerrainShape> _gravel_shape;
    Ref<PlanetTerrainShape> _sand_shape;
    Ref<PlanetTerrainShape> _snow_shape;
    Ref<PlanetTerrainShape> _stone_shape;
    Ref<PlanetTerrainShape> _water_shape;

protected:
    static void _bind_methods();

public:
    PlanetShape();
    virtual ~PlanetShape() = default;

    void set_center(const Vector3& c) { _center = c; notify_shape_changed(); }
    Vector3 get_center() const { return _center; }

    void set_radius(float r) { _radius = r; notify_shape_changed(); }
    float get_radius() const { return _radius; }

    void set_amplitude(float a) { _amplitude = a; notify_shape_changed(); }
    float get_amplitude() const { return _amplitude; }

    StringName get_shape_type() const override { return "planet"; }
    /// @brief Базовый SDF планеты: dist(p - center) - (radius + noise * amplitude).
    float evaluate_sdf(const Vector3& world_pos) const override;
    void recompute_aabb() override;
    void on_register() override;
};

} // namespace godot
