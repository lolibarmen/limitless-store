#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {

/// @brief Генератор материала ветки: пиксель-арт текстура с запечённым наклоном (lean).
class BranchMaterialGenerator : public MaterialGenerator {
    GDCLASS(BranchMaterialGenerator, MaterialGenerator)

protected:
    static void _bind_methods();

public:
    BranchMaterialGenerator() = default;
    virtual ~BranchMaterialGenerator() = default;

    Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
