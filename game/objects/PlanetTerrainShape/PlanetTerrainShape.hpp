#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class PlanetShape;

/// @brief Материал планеты: часть поверхности PlanetShape, помеченная этим material_id
/// в общем воронои-шуме планеты (см. PlanetShape::evaluate_sdf и bind_material).
/// Сам SDF не вычисляет — единственный источник SDF — PlanetShape: вне «своей» ячейки
/// воронои возвращает INFINITY, внутри — базовый SDF планеты.
class PlanetTerrainShape : public SemanticShape {
    GDCLASS(PlanetTerrainShape, SemanticShape)

public:
    /// @brief Количество материалов планеты (материалы получают id 0..kMaterialCount-1).
    static constexpr int kMaterialCount = 7;

private:
    Vector3 _center;
    float _radius = 100.0f;
    float _amplitude = 20.0f;

    // Собственный шум больше не нужен для SDF: остаётся только как источник высот
    // для генераторов геометрии/материалов.
    Ref<FastNoiseLite> _noise;
    Ref<SurfaceGenerator> _generator;
    Ref<TriplanarMaterialGenerator> _material_generator;
    String _albedo_texture = "res://assets/Chunk/grass.png";

    // Общий воронои-шум планеты, задающий границы материалов (привязывается через bind_material).
    Ref<FastNoiseLite> _shared_voronoi;
    int _this_material_id = -1;

    // Родительская планета — единственный источник базового SDF (привязывается через bind_material).
    Ref<PlanetShape> _planet;

protected:
    static void _bind_methods();

public:
    PlanetTerrainShape();
    virtual ~PlanetTerrainShape() = default;

    /// @brief Путь к текстуре albedo, загружается в on_register().
    void set_albedo_texture(const String& path) { _albedo_texture = path; }

    void set_center(const Vector3& c) { _center = c; notify_shape_changed(); }
    Vector3 get_center() const { return _center; }

    void set_radius(float r) { _radius = r; notify_shape_changed(); }
    float get_radius() const { return _radius; }

    void set_amplitude(float a) { _amplitude = a; notify_shape_changed(); }
    float get_amplitude() const { return _amplitude; }

    /// @brief Привязывает фигуру к планете-родителю (единственный источник базового SDF),
    /// общему воронои-шуму планеты и id материала этой фигуры в распределении Вороного.
    void bind_material(const Ref<PlanetShape>& planet, const Ref<FastNoiseLite>& shared_voronoi, int material_id) {
        _planet = planet;
        _shared_voronoi = shared_voronoi;
        _this_material_id = material_id;
    }

    /// @brief Id материала (ячейки общего воронои-распределения планеты) для точки мира.
    /// Значение CELLULAR-шума — расстояние до ближайшей особенности (непрерывное), поэтому
    /// id получается квантованием значения шума по kMaterialCount, а не сравнением «напрямую».
    int material_at(const Vector3& world_pos) const;

    StringName get_shape_type() const override { return "planet_terrain"; }
    float evaluate_sdf(const Vector3& world_pos) const override;
    void recompute_aabb() override;
    void on_zone_changed(uint64_t chunk_id) override;
    void on_register() override;
};

} // namespace godot
