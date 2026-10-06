#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/color.hpp>

namespace godot {

class TriplanarMaterialGenerator : public MaterialGenerator {
    GDCLASS(TriplanarMaterialGenerator, MaterialGenerator)

private:
    Ref<Texture2D> _albedo_texture;
    Color _albedo_color = Color(1.0, 1.0, 1.0, 1.0);
    float _texture_scale = 1.0f;
    float _roughness = 0.8f;
    bool _use_normal_map = false;
    Ref<Texture2D> _normal_texture;

protected:
    static void _bind_methods();

public:
    TriplanarMaterialGenerator() = default;
    virtual ~TriplanarMaterialGenerator() = default;

    // Геттеры и сеттеры для свойств, доступных в инспекторе Godot
    void set_albedo_texture(const Ref<Texture2D>& p_texture) { _albedo_texture = p_texture; }
    Ref<Texture2D> get_albedo_texture() const { return _albedo_texture; }

    void set_albedo_color(const Color& p_color) { _albedo_color = p_color; }
    Color get_albedo_color() const { return _albedo_color; }

    void set_texture_scale(float p_scale) { _texture_scale = p_scale; }
    float get_texture_scale() const { return _texture_scale; }

    void set_roughness(float p_roughness) { _roughness = p_roughness; }
    float get_roughness() const { return _roughness; }

    void set_use_normal_map(bool p_use) { _use_normal_map = p_use; }
    bool get_use_normal_map() const { return _use_normal_map; }

    void set_normal_texture(const Ref<Texture2D>& p_texture) { _normal_texture = p_texture; }
    Ref<Texture2D> get_normal_texture() const { return _normal_texture; }

    // Переопределение виртуального метода генерации
    virtual Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot