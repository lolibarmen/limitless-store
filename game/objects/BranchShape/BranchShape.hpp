#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <BranchMeshGenerator/BranchMeshGenerator.hpp>
#include <BranchMaterialGenerator/BranchMaterialGenerator.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

/// @brief Ветка: прямоугольная карточка вдоль локального верха планеты; наклон запечён в текстуру.
class BranchShape : public SemanticShape {
    GDCLASS(BranchShape, SemanticShape)

private:
    Ref<PlanetShape> _planet;

    Vector3 _position;
    Vector3 _direction = Vector3(0.0f, 1.0f, 0.0f);
    float _length = 1.5f;
    float _width = 0.3f;

    Ref<BranchMeshGenerator> _mesh_generator;
    Ref<BranchMaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

public:
    BranchShape();
    virtual ~BranchShape() = default;

    void bind_planet(const Ref<PlanetShape>& planet);

    void set_position(const Vector3& v) { _position = v; notify_shape_changed(); }
    Vector3 get_position() const { return _position; }

    void set_direction(const Vector3& v);
    Vector3 get_direction() const { return _direction; }

    void set_length(float v) { _length = v; notify_shape_changed(); }
    float get_length() const { return _length; }

    void set_width(float v) { _width = v; notify_shape_changed(); }
    float get_width() const { return _width; }

    Vector3 up() const;         // локальный верх планеты в точке крепления
    Vector3 side() const;       // боковая ось карточки (перпендикулярна up)
    float card_height() const;  // проекция length на up
    float card_width() const;   // width + |lean|

    StringName get_shape_type() const override { return "branch"; }
    void recompute_aabb() override;
    void on_register() override;
    void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot
