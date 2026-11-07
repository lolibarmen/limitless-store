#pragma once

#include <MaterialGenerator/MaterialGenerator.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {

/// @brief Генератор материала ветки: процедурная пиксель-арт текстура коры.
class BranchMaterialGenerator : public MaterialGenerator {
    GDCLASS(BranchMaterialGenerator, MaterialGenerator)

private:
    float _pixel_size = 0.025f;
    int _max_depth = 7;

protected:
    static void _bind_methods();

public:
    BranchMaterialGenerator() = default;
    virtual ~BranchMaterialGenerator() = default;

    void set_pixel_size(float v) { _pixel_size = v > 0.0f ? v : 0.025f; }
    float get_pixel_size() const { return _pixel_size; }

    void set_max_depth(int v) { _max_depth = v > 0 ? v : 1; }
    int get_max_depth() const { return _max_depth; }

    Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
