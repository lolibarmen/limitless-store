#pragma once

#include <SemanticSphere/SemanticSphere.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class TestSphere : public SemanticSphere {
    GDCLASS(TestSphere, SemanticSphere)

private:

    Ref<SurfaceGenerator> _generator_lod0;
    Ref<SurfaceGenerator> _generator_lod1;
    Ref<SurfaceGenerator> _generator_lod2;

protected:
    static void _bind_methods();

public:
    TestSphere();
    virtual ~TestSphere() = default;

    String get_shape_type() const override { return "stone_sphere"; }

    virtual Ref<MeshGenerator> get_mesh_generator() const override;

    virtual void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot