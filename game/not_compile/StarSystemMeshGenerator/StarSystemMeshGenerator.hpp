#pragma once
#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
class StarSystemMeshGenerator : public MeshGenerator {
    GDCLASS(StarSystemMeshGenerator, MeshGenerator)
protected:
    static void _bind_methods();
private:
    Vector3 _pos = Vector3();
    float _radius = 10.0f;

public:
    StarSystemMeshGenerator() = default;
    void set_pos(Vector3 p) { _pos = p; }
    Vector3 get_pos() const { return _pos; }
    void set_radius(float r) { _radius = r; }
    float get_radius() const { return _radius; }
    virtual Ref<ArrayMesh> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};
} // namespace godot