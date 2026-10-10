#pragma once

#include <SemanticSphere/SemanticSphere.hpp>
#include <CubeMeshGenerator/CubeMeshGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class TestCube : public SemanticShape {
    GDCLASS(TestCube, SemanticShape)

private:

    Ref<CubeMeshGenerator> _mesh_generator;
    Ref<TriplanarMaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

    Vector3 _pos = {};
    Vector3 _size = {};

public:
    TestCube();
    virtual ~TestCube() {};

    String get_shape_type() const override { return "test_cube"; }

    void recompute_aabb();

    virtual void on_zone_changed(uint64_t chunk_id) override;

    void set_pos(Vector3 p_pos) { _pos = p_pos; }
    Vector3 get_pos() const { return _pos; }

    void set_size(Vector3 p_size) { _size = p_size; }
    Vector3 get_size() const { return _size; }
};

} // namespace godot