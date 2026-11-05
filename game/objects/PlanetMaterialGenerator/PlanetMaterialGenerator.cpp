#include "PlanetMaterialGenerator.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {

void PlanetMaterialGenerator::_bind_methods() {
    // Минимальная реализация не требует проброса свойств в инспектор, 
    // так как текстуры жестко заданы архитектурой планеты.
}

PlanetMaterialGenerator::PlanetMaterialGenerator() {
    _shader.instantiate();
    
    // Кастомный шейдер: World Triplanar + Чтение биома из Vertex Color (с фолбэком на процедурный шум)
    String shader_code = R"(
        shader_type spatial;
        render_mode blend_mix, depth_draw_opaque, cull_back;
        
        uniform sampler2D tex0 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex1 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex2 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex3 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex4 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex5 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex6 : source_color, filter_linear_mipmap, repeat_enable;
        uniform sampler2D tex7 : source_color, filter_linear_mipmap, repeat_enable;
        
        uniform float texture_scale = 0.1;
        
        varying vec3 world_pos;
        varying vec3 world_normal;
        varying flat vec4 flat_color;
        
        vec4 sample_triplanar(int id, vec3 pos, vec3 blend) {
            vec4 x, y, z;
            if (id == 0) { x = texture(tex0, pos.yz); y = texture(tex0, pos.xz); z = texture(tex0, pos.xy); }
            else if (id == 1) { x = texture(tex1, pos.yz); y = texture(tex1, pos.xz); z = texture(tex1, pos.xy); }
            else if (id == 2) { x = texture(tex2, pos.yz); y = texture(tex2, pos.xz); z = texture(tex2, pos.xy); }
            else if (id == 3) { x = texture(tex3, pos.yz); y = texture(tex3, pos.xz); z = texture(tex3, pos.xy); }
            else if (id == 4) { x = texture(tex4, pos.yz); y = texture(tex4, pos.xz); z = texture(tex4, pos.xy); }
            else if (id == 5) { x = texture(tex5, pos.yz); y = texture(tex5, pos.xz); z = texture(tex5, pos.xy); }
            else if (id == 6) { x = texture(tex6, pos.yz); y = texture(tex6, pos.xz); z = texture(tex6, pos.xy); }
            else { x = texture(tex7, pos.yz); y = texture(tex7, pos.xz); z = texture(tex7, pos.xy); }
            
            return x * blend.x + y * blend.y + z * blend.z;
        }
        
        void vertex() {
            world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
            world_normal = normalize((MODEL_MATRIX * vec4(NORMAL, 0.0)).xyz);
            flat_color = COLOR; // <-- КОПИРУЕМ COLOR БЕЗ ИНТЕРПОЛЯЦИИ
        }
        
        void fragment() {
            vec3 blend = abs(world_normal);
            blend = normalize(max(blend, 0.00001));
            float b = blend.x + blend.y + blend.z;
            blend /= b;
            
            vec3 pos = world_pos * texture_scale;
            
            // Читаем из flat_color вместо COLOR
            int id = int(round(flat_color.r * 7.0));
            id = clamp(id, 0, 7);
            
            vec4 final_color = sample_triplanar(id, pos, blend);
            ALBEDO = final_color.rgb;
        }
)";
    _shader->set_code(shader_code);

    // Загрузка текстур (предполагаем, что они лежат в res:// или res://textures/)
    const char* tex_names[] = { "dirt", "grass", "gravel", "null", "sand", "snow", "stone", "water" };
    for (int i = 0; i < 8; ++i) {
        String path = String("res://assets/Chunk/") + tex_names[i] + ".webp";
        Ref<Texture2D> tex = ResourceLoader::get_singleton()->load(path);
        _textures[i] = tex;
    }
}

Ref<Material> PlanetMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    Ref<ShaderMaterial> material;
    material.instantiate();
    material->set_shader(_shader);
    
    // Передача текстур в шейдер
    for (int i = 0; i < 8; ++i) {
        StringName param_name = StringName("tex") + String::num_int64(i);
        if (_textures[i].is_valid()) {
            material->set_shader_parameter(param_name, _textures[i]);
        }
    }
    
    material->set_shader_parameter("texture_scale", 2.0f);
    material->set_shader_parameter("biome_scale", 0.05f);
    
    return material;
}

} // namespace godot