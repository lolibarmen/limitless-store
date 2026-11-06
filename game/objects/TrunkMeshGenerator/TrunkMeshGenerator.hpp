#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

/// @brief Генератор меша ствола: низкополигональный конус из колец.
class TrunkMeshGenerator : public MeshGenerator {
    GDCLASS(TrunkMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    TrunkMeshGenerator() = default;
    virtual ~TrunkMeshGenerator() = default;

    void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
