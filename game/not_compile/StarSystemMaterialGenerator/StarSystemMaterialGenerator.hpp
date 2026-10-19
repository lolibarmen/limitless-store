#pragma once
#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/material.hpp>

namespace godot {
class StarSystemMaterialGenerator : public MaterialGenerator {
    GDCLASS(StarSystemMaterialGenerator, MaterialGenerator)
protected:
    static void _bind_methods();
public:
    StarSystemMaterialGenerator() = default;
    virtual Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};
} // namespace godot