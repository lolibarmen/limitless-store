#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

/// @brief Генератор меша ветки: один прямоугольник с UV вдоль верха планеты.
class BranchMeshGenerator : public MeshGenerator {
    GDCLASS(BranchMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    BranchMeshGenerator() = default;
    virtual ~BranchMeshGenerator() = default;

    void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
