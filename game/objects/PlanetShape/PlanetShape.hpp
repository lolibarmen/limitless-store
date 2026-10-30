#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

namespace godot {

class PlanetTerrainShape;

/// @brief Планета, состоящая из нескольких PlanetTerrainShape: каждая получает свою текстуру материала.
/// Фигуры создаются и регистрируются в SemanticWorld при регистрации самой планеты (on_register)
/// и пространственно привязываются к октодереву первой из них.
class PlanetShape : public SemanticShape {
    GDCLASS(PlanetShape, SemanticShape)

private:
    Vector3 _center;
    float _radius = 100.0f;
    float _amplitude = 20.0f;

    /// Текстуры материалов фигур планеты (по одной PlanetTerrainShape на текстуру).
    PackedStringArray _textures = { "res://assets/Chunk/grass.png", "res://assets/Chunk/stone.png" };

    /// Созданные фигуры — нужны, чтобы задать им параметры и отвязать при удалении планеты.
    std::vector<Ref<PlanetTerrainShape>> _parts;

protected:
    static void _bind_methods();

public:
    PlanetShape() = default;
    virtual ~PlanetShape() = default;

    void set_center(const Vector3& c) { _center = c; notify_shape_changed(); }
    Vector3 get_center() const { return _center; }

    void set_radius(float r) { _radius = r; notify_shape_changed(); }
    float get_radius() const { return _radius; }

    void set_amplitude(float a) { _amplitude = a; notify_shape_changed(); }
    float get_amplitude() const { return _amplitude; }

    void set_textures(const PackedStringArray& textures) { _textures = textures; }
    PackedStringArray get_textures() const { return _textures; }

    StringName get_shape_type() const override { return "planet"; }
    void recompute_aabb() override;
    void on_register() override;
};

} // namespace godot
