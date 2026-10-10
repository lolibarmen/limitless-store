#include "TriplanarMaterialGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace godot {

void TriplanarMaterialGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_albedo_texture", "texture"), &TriplanarMaterialGenerator::set_albedo_texture);
    ClassDB::bind_method(D_METHOD("get_albedo_texture"), &TriplanarMaterialGenerator::get_albedo_texture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "albedo_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_albedo_texture", "get_albedo_texture");

    ClassDB::bind_method(D_METHOD("set_albedo_color", "color"), &TriplanarMaterialGenerator::set_albedo_color);
    ClassDB::bind_method(D_METHOD("get_albedo_color"), &TriplanarMaterialGenerator::get_albedo_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "albedo_color"), "set_albedo_color", "get_albedo_color");

    ClassDB::bind_method(D_METHOD("set_texture_scale", "scale"), &TriplanarMaterialGenerator::set_texture_scale);
    ClassDB::bind_method(D_METHOD("get_texture_scale"), &TriplanarMaterialGenerator::get_texture_scale);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "texture_scale", PROPERTY_HINT_RANGE, "0.01,100.0,0.01"), "set_texture_scale", "get_texture_scale");

    ClassDB::bind_method(D_METHOD("set_roughness", "roughness"), &TriplanarMaterialGenerator::set_roughness);
    ClassDB::bind_method(D_METHOD("get_roughness"), &TriplanarMaterialGenerator::get_roughness);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "roughness", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_roughness", "get_roughness");

    ClassDB::bind_method(D_METHOD("set_use_normal_map", "enable"), &TriplanarMaterialGenerator::set_use_normal_map);
    ClassDB::bind_method(D_METHOD("get_use_normal_map"), &TriplanarMaterialGenerator::get_use_normal_map);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_normal_map"), "set_use_normal_map", "get_use_normal_map");

    ClassDB::bind_method(D_METHOD("set_normal_texture", "texture"), &TriplanarMaterialGenerator::set_normal_texture);
    ClassDB::bind_method(D_METHOD("get_normal_texture"), &TriplanarMaterialGenerator::get_normal_texture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_normal_texture", "get_normal_texture");
}

Ref<Material> TriplanarMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<StandardMaterial3D> material;
    material.instantiate();

    material->set_albedo(_albedo_color);
    if (_albedo_texture.is_valid()) {
        material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, _albedo_texture);
    }

    material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS);

    material->set_flag(BaseMaterial3D::FLAG_UV1_USE_TRIPLANAR, true);
    material->set_flag(BaseMaterial3D::FLAG_UV1_USE_WORLD_TRIPLANAR, true);

    material->set_uv1_triplanar_blend_sharpness(10);

    float scale = _texture_scale > 0.0f ? _texture_scale : 1.0f;
    material->set_uv1_scale(Vector3(scale, scale, scale));

    material->set_roughness(_roughness);

    if (_use_normal_map && _normal_texture.is_valid()) {
        material->set_texture(BaseMaterial3D::TEXTURE_NORMAL, _normal_texture);
        material->set_feature(BaseMaterial3D::FEATURE_NORMAL_MAPPING, true);
        material->set_normal_scale(1.0f);
    }

    return material;
}

} // namespace godot