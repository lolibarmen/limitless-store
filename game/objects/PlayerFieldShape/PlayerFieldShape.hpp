#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <SphereMeshGenerator/SphereMeshGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class PlayerFieldShape : public SemanticShape {
    GDCLASS(PlayerFieldShape, SemanticShape)

private:
    float _area_radius = 2.0f;
    Ref<SphereMeshGenerator> _area_generator;

protected:
    static void _bind_methods();

public:
    PlayerFieldShape();
    virtual ~PlayerFieldShape() = default;

    void set_area_radius(float r);
    float get_area_radius() const { return _area_radius; }

    StringName get_shape_type() const override { return "player_field"; }
    void recompute_aabb() override;
    void on_register() override;
    void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot
