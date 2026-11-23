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
    mutable uint64_t _pending_gravity_id = 0;
    uint64_t _bound_gravity_id = 0;

protected:
    static void _bind_methods();

public:
    PlayerFieldShape();
    virtual ~PlayerFieldShape() = default;

    void set_area_radius(float r);
    float get_area_radius() const { return _area_radius; }

    uint64_t get_bound_gravity_id() const { return _bound_gravity_id; }
    uint64_t consume_pending_gravity_id();

    StringName get_shape_type() const override { return "player_field"; }
    void recompute_aabb() override;
    void on_register() override;
    void on_zone_changed(uint64_t chunk_id) override;
    Ref<SemanticShape> evaluate_alchemy(uint64_t other_id) const override;
    void on_alchemy_separation(uint64_t other_id) override;
};

} // namespace godot
