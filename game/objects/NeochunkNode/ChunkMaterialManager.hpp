#pragma once

#include <godot_cpp/classes/shader_material.hpp>

namespace godot {

class ChunkMaterialManager {
public:
    static ChunkMaterialManager& get_singleton() {
        static ChunkMaterialManager instance;
        return instance;
    }

    void initialize(); // вызвать один раз при старте
    Ref<ShaderMaterial> get_material() const { return _material; }

private:
    Ref<ShaderMaterial> _material;
};

} // namespace godot