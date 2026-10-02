#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/aabb.hpp>

namespace godot {

class SemanticShape;

class MeshGenerator : public Resource {
    GDCLASS(MeshGenerator, Resource)

protected:
    static void _bind_methods();

public:
    MeshGenerator() = default;
    ~MeshGenerator() = default;

    virtual Ref<ArrayMesh> generate(uint64_t shape_id, uint64_t chunk_id) const;
};

} // namespace godot