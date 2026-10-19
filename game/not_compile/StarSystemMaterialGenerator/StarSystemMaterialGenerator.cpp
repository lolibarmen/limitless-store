#include "StarSystemMaterialGenerator.hpp"
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

void StarSystemMaterialGenerator::_bind_methods() {}

Ref<Material> StarSystemMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<StandardMaterial3D> mat;
    mat.instantiate();
    
    // Полупрозрачная, светящаяся сфера, видимая с обеих сторон
    mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
    mat->set_albedo(Color(0.6, 0.8, 1.0, 0.35)); 
    mat->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
    mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    
    return mat;
}