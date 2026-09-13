#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

namespace godot {

class SemanticShape;

class MeshGenerator : public Resource {
    GDCLASS(MeshGenerator, Resource)

protected:
    static void _bind_methods();

public:
    MeshGenerator() = default;
    ~MeshGenerator() = default;

    virtual Ref<ArrayMesh> generate(Ref<SemanticShape> shape) const;
};

} // namespace godot