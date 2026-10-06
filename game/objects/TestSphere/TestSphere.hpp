#pragma once

#include <SemanticSphere/SemanticSphere.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class SurfaceGenerator;
class MaterialGenerator;

class TestSphere : public SemanticSphere {
    GDCLASS(TestSphere, SemanticSphere)

private:

    Ref<SurfaceGenerator> _mesh_generator_lod0;
    Ref<SurfaceGenerator> _mesh_generator_lod1;
    Ref<SurfaceGenerator> _mesh_generator_lod2;

    Ref<MaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

public:
    TestSphere();
    virtual ~TestSphere() = default;

    String get_shape_type() const override { return "stone_sphere"; }

    virtual void on_zone_changed(uint64_t chunk_id) override;
};

} // namespace godot