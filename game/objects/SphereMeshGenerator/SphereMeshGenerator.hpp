#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

class SphereMeshGenerator : public MeshGenerator {
    GDCLASS(SphereMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    SphereMeshGenerator() = default;
    virtual ~SphereMeshGenerator() = default;

    void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot
