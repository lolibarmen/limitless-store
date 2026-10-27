#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

class CubeMeshGenerator : public MeshGenerator {
    GDCLASS(CubeMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    CubeMeshGenerator() = default;
    ~CubeMeshGenerator() = default;

    virtual void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot