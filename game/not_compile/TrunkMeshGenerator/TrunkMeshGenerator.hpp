#pragma once

#include <MeshGenerator/MeshGenerator.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

class TrunkMeshGenerator : public MeshGenerator {
    GDCLASS(TrunkMeshGenerator, MeshGenerator)

protected:
    static void _bind_methods();

private:
    int radial_segments = 8;
    int height_segments = 6;
    float taper = 0.3f;       // радиус_верх / радиус_низ
    float bend_amount = 0.4f; // макс. смещение оси изгибом

public:
    TrunkMeshGenerator() = default;
    ~TrunkMeshGenerator() = default;

    void set_radial_segments(int v) { radial_segments = MAXI(v, 3); }
    int get_radial_segments() const { return radial_segments; }

    void set_height_segments(int v) { height_segments = MAXI(v, 1); }
    int get_height_segments() const { return height_segments; }

    void set_taper(float v) { taper = CLAMPF(v, 0.01f, 1.0f); }
    float get_taper() const { return taper; }

    void set_bend_amount(float v) { bend_amount = MAXF(v, 0.0f); }
    float get_bend_amount() const { return bend_amount; }

    virtual void generate(uint64_t shape_id, uint64_t chunk_id) const override;
};

} // namespace godot