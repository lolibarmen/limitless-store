#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <SphereMeshGenerator/SphereMeshGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <unordered_set>

namespace godot {

class SurfaceShape;

class ForestShape : public SemanticShape {
    GDCLASS(ForestShape, SemanticShape)

private:
    Ref<PlanetShape> _planet;
    Ref<SemanticShape> _tree_template;
    Ref<SphereMeshGenerator> _area_generator;

    float _radius = 8.0f;
    int _tree_count = 12;

    uint64_t _planted_surface_id = 0;
    std::unordered_set<uint64_t> _planted_surfaces;

    void _plant_trees(SurfaceShape* surface);
    void _clear_trees();
    void _replant();

protected:
    static void _bind_methods();

public:
    ForestShape();
    virtual ~ForestShape() = default;

    void bind_planet(const Ref<PlanetShape>& planet);
    Ref<PlanetShape> get_planet() const { return _planet; }

    void set_tree_shape(const Ref<SemanticShape>& shape);
    Ref<SemanticShape> get_tree_shape() const { return _tree_template; }

    void set_radius(float r);
    float get_radius() const { return _radius; }
    float get_area_radius() const { return _radius; }

    void set_tree_count(int n);
    int get_tree_count() const { return _tree_count; }

    StringName get_shape_type() const override { return "forest"; }
    void recompute_aabb() override;
    void on_register() override;
    void on_unregister() override;
    void on_zone_changed(uint64_t chunk_id) override;

    Ref<SemanticShape> evaluate_alchemy(uint64_t other_id) const override;
};

} // namespace godot
