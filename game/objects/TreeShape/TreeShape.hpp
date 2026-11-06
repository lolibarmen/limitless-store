#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <TrunkShape/TrunkShape.hpp>
#include <BranchShape/BranchShape.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Дерево — фигура-владелец: создаёт ствол и несколько веток.
///
/// Своего октодерева и собственного меша нет: TreeShape, TrunkShape и BranchShape
/// привязываются к октодереву планеты (bind_spatially). Состав задаётся через
/// add_owned_shape: при регистрации создаются дочерние TrunkShape и BranchShape.
///
/// Позиции веток на стволе назначаются здесь: точки крепления берутся с центральной
/// линии ствола (TrunkShape::centerline), направление — касательная ствола,
/// отклонённая на угол spread в сторону азимута (равномерно вокруг ствола
/// с детерминированным разбросом из seed).
class TreeShape : public SemanticShape {
    GDCLASS(TreeShape, SemanticShape)

private:
    Ref<PlanetShape> _planet;

    Vector3 _origin;
    Vector3 _up = Vector3(0.0f, 1.0f, 0.0f);
    uint64_t _seed = 0;

    // Параметры ствола (переносятся в дочерний TrunkShape).
    float _height = 8.0f;
    float _base_radius = 0.5f;
    float _top_radius = 0.18f;
    Vector3 _bend;
    int _radial_segments = 7;
    int _height_segments = 6;
    float _ring_variation = 0.15f;

    // Параметры веток.
    int _branch_count = 5;
    float _branch_start = 0.45f;
    float _branch_end = 0.95f;
    float _branch_length = 2.0f;
    float _branch_width = 0.35f;
    float _branch_spread = 0.7f;

    uint64_t _trunk_id = 0;

    void _spawn_children();

protected:
    static void _bind_methods();

public:
    TreeShape() = default;
    virtual ~TreeShape() = default;

    void bind_planet(const Ref<PlanetShape>& planet);

    void set_origin(const Vector3& v) { _origin = v; notify_shape_changed(); }
    Vector3 get_origin() const { return _origin; }

    void set_up(const Vector3& v);
    Vector3 get_up() const { return _up; }

    void set_seed(uint64_t v) { _seed = v; notify_shape_changed(); }
    uint64_t get_seed() const { return _seed; }

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

    void set_branch_count(int v) { _branch_count = v; notify_shape_changed(); }
    int get_branch_count() const { return _branch_count; }

    void set_branch_start(float v) { _branch_start = v; notify_shape_changed(); }
    float get_branch_start() const { return _branch_start; }

    void set_branch_end(float v) { _branch_end = v; notify_shape_changed(); }
    float get_branch_end() const { return _branch_end; }

    void set_branch_length(float v) { _branch_length = v; notify_shape_changed(); }
    float get_branch_length() const { return _branch_length; }

    void set_branch_width(float v) { _branch_width = v; notify_shape_changed(); }
    float get_branch_width() const { return _branch_width; }

    void set_branch_spread(float v) { _branch_spread = v; notify_shape_changed(); }
    float get_branch_spread() const { return _branch_spread; }

    /// @brief ID дочернего ствола (0 — ещё не создан).
    uint64_t get_trunk_id() const { return _trunk_id; }

    StringName get_shape_type() const override { return "tree"; }
    void recompute_aabb() override;
    void on_register() override;
};

} // namespace godot
