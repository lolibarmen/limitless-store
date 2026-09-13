#pragma once

#include <SemanticSphere/SemanticSphere.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class StoneSphere : public SemanticSphere {
    GDCLASS(StoneSphere, SemanticSphere)

private:

    Ref<SurfaceGenerator> _generator;

protected:
    static void _bind_methods();

public:
    StoneSphere();
    virtual ~StoneSphere() = default;

    String get_shape_type() const override { return "stone_sphere"; }

    virtual Ref<MeshGenerator> get_mesh_generator() const override;
};

} // namespace godot