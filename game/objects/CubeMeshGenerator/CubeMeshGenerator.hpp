#pragma once

#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

class CubeMeshGenerator : public MeshGenerator {
    GDCLASS(CubeMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

private:
    Vector3 pos = {0.0, 0.0, 0.0};
    Vector3 size = {0.0, 0.0, 0.0};

public:
    CubeMeshGenerator();
    ~CubeMeshGenerator();

    void set_pos(Vector3 p_pos) { pos = p_pos; }
    Vector3 get_pos() const { return pos; }

    void set_size(Vector3 p_size) { size = p_size; }
    Vector3 get_size() const { return size; }

    virtual Ref<ArrayMesh> generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot