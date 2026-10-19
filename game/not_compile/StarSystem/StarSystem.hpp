#pragma once
#include <SemanticShape/SemanticShape.hpp>
#include <StarSystemMeshGenerator/StarSystemMeshGenerator.hpp>
#include <StarSystemMaterialGenerator/StarSystemMaterialGenerator.hpp>

namespace godot {
class StarSystem : public SemanticShape {
    GDCLASS(StarSystem, SemanticShape)
private:
    Ref<StarSystemMeshGenerator> _mesh_generator;
    Ref<StarSystemMaterialGenerator> _material_generator;
    Vector3 _pos = Vector3();
    float _radius = 1000.0f;

protected:
    static void _bind_methods();

public:
    StarSystem();
    virtual ~StarSystem() = default;

    StringName get_shape_type() const override { return StringName("star_system"); }
    void recompute_aabb() override;
    void on_zone_changed(uint64_t chunk_id) override;

    void set_pos(Vector3 p) { _pos = p; }
    Vector3 get_pos() const { return _pos; }
    void set_radius(float r) { _radius = r; }
    float get_radius() const { return _radius; }
};
} // namespace godot