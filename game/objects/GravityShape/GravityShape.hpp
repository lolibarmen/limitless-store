#pragma once

#include <GravityAreaGenerator/GravityAreaGenerator.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class GravityShape : public SemanticShape {
    GDCLASS(GravityShape, SemanticShape)

private:
    Ref<PlanetShape> _planet;
    Ref<GravityAreaGenerator> _area_generator;

protected:
    static void _bind_methods();

public:
    GravityShape();
    virtual ~GravityShape() = default;

    void bind_planet(const Ref<PlanetShape>& planet);
    Ref<PlanetShape> get_planet() const { return _planet; }

    Vector3 get_up_at(const Vector3& local_pos) const;
    float get_area_radius() const;

    StringName get_shape_type() const override { return "gravity"; }
    void recompute_aabb() override;
    void on_register() override;
    void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot
