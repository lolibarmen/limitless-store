#pragma once

#include "SurfaceNets.hpp"
#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <memory>

namespace godot {

class SurfaceGenerator : public MeshGenerator {
    GDCLASS(SurfaceGenerator, MeshGenerator)

private:
    int _lod_level = 0;
    float _voxel_count = 16.0f;

    static void _build_mesh_task(uint64_t generator_id, uint64_t shape_id);
    void _on_mesh_generated(uint64_t shape_id, Ref<ArrayMesh> mesh);

protected:
    static void _bind_methods();

public:
    SurfaceGenerator() = default;
    ~SurfaceGenerator() override = default;

    virtual Ref<ArrayMesh> generate(Ref<SemanticShape> shape) const override;

    void set_lod_level(int level) { _lod_level = level; }
    int get_lod_level() const { return _lod_level; }

    void set_voxel_count(float count) { _voxel_count = count; }
    float get_voxel_count() const { return _voxel_count; }
};

} // namespace godot