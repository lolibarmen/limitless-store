#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/texture2d.hpp>

namespace godot {

class PlanetMaterialGenerator : public MaterialGenerator {
    GDCLASS(PlanetMaterialGenerator, MaterialGenerator)

private:
    Ref<Shader> _shader;
    Ref<Texture2D> _textures[8];

protected:
    static void _bind_methods();

public:
    PlanetMaterialGenerator();
    virtual ~PlanetMaterialGenerator() = default;

    virtual Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot