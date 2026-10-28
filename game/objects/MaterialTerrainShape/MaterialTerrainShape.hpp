#pragma once

#include <SemanticShape/SemanticShape.hpp>
#include <SurfaceGenerator/SurfaceGenerator.hpp>
#include <TriplanarMaterialGenerator/TriplanarMaterialGenerator.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {

class MaterialTerrainShape : public SemanticShape {
    GDCLASS(MaterialTerrainShape, SemanticShape)

private:
    String _material_type = "dirt";
    float _base_height = 0.0f;
    float _amplitude = 20.0f;
    float _frequency = 0.05f;
    int _seed = 12345;
    
    mutable Ref<FastNoiseLite> _noise;

    Ref<SurfaceGenerator> _generator_lod0;
    Ref<SurfaceGenerator> _generator_lod1;
    Ref<SurfaceGenerator> _generator_lod2;

    Ref<TriplanarMaterialGenerator> _material_generator;

protected:
    static void _bind_methods();

public:
    MaterialTerrainShape();
    virtual ~MaterialTerrainShape() = default;

    void set_material_type(const String& type) { _material_type = type; }
    String get_material_type() const { return _material_type; }

    void set_base_height(float h) { _base_height = h; }
    float get_base_height() const { return _base_height; }

    void set_amplitude(float a) { _amplitude = a; }
    float get_amplitude() const { return _amplitude; }

    void set_frequency(float f) { 
        _frequency = f; 
        if (_noise.is_valid()) _noise->set_frequency(_frequency);
    }
    float get_frequency() const { return _frequency; }

    void set_seed(int s) { 
        _seed = s; 
        if (_noise.is_valid()) _noise->set_seed(_seed);
    }
    int get_seed() const { return _seed; }

    StringName get_shape_type() const override { return "material_terrain"; }
    
    // Этот метод будет вызываться SurfaceGenerator для построения сетки
    float evaluate_sdf(const Vector3& world_pos) const override;
    
    virtual void on_zone_changed(uint64_t chunk_id) override;
    
    void recompute_aabb() override;
};

} // namespace godot