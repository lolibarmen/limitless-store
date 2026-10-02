#pragma once

#include <MeshGenerator/MeshGenerator.hpp>

namespace godot {

class AABBBoxGenerator : public MeshGenerator {
    GDCLASS(AABBBoxGenerator, MeshGenerator)

protected:
    static void _bind_methods();

public:
    Ref<ArrayMesh> generate(uint64_t shape_id, const AABB& bounds) const override;
};

} // namespace godot