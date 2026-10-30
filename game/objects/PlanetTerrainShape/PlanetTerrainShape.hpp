#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Планета с ландшафтом: сфера SphericalTerrainShape + высоты (шум) MaterialTerrainShape.
/// SDF: dist(p - center) - (radius + noise * amplitude).
class PlanetTerrainShape : public SemanticShape {
    GDCLASS(PlanetTerrainShape, SemanticShape)

private:
    Vector3 _center;
    float _radius = 100.0f;
    float _amplitude = 20.0f;

    Ref<FastNoiseLite> _noise;
    Ref<SurfaceGenerator> _generator;
    Ref<TriplanarMaterialGenerator> _material_generator;
    String _texture_path = "res://assets/Chunk/grass.png";
    uint64_t _octree_id = 0;

protected:
    static void _bind_methods();

public:
    PlanetTerrainShape();
    virtual ~PlanetTerrainShape() = default;

    void set_center(const Vector3& c) { _center = c; notify_shape_changed(); }
    Vector3 get_center() const { return _center; }

    void set_radius(float r) { _radius = r; notify_shape_changed(); }
    float get_radius() const { return _radius; }

    void set_amplitude(float a) { _amplitude = a; notify_shape_changed(); }
    float get_amplitude() const { return _amplitude; }

    /// @brief Задаёт текстуру альбедо материала фигуры (например трава или камень).
    void set_texture_path(const String& path) { _texture_path = path; }
    String get_texture_path() const { return _texture_path; }

    /// @brief Задаёт октодерево фигуры: если задан ненулевой ID, фигура использует это дерево,
    /// иначе создаёт собственное (по умолчанию — в on_register()).
    void set_octree_id(uint64_t id) { _octree_id = id; }
    uint64_t get_octree_id() const { return _octree_id; }

    StringName get_shape_type() const override { return "planet_terrain"; }
    float evaluate_sdf(const Vector3& world_pos) const override;
    void recompute_aabb() override;
    void on_zone_changed(uint64_t chunk_id) override;
    void on_register() override;
};

} // namespace godot
