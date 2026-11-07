#include "BranchMaterialGenerator.hpp"
#include <BranchShape/BranchShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <Utils/RandomUtils.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <cmath>

using namespace godot;

void BranchMaterialGenerator::_bind_methods() {}

Ref<Material> BranchMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    float width = 1.0f;
    float card_width = 1.0f;
    float card_height = 1.0f;
    float lean = 0.0f;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    Ref<SemanticShape> base = sw->get_shape(shape_id);
    Ref<BranchShape> shape = Object::cast_to<BranchShape>(base.ptr());
    if (shape.is_valid()) {
        width = shape->get_width();
        card_width = shape->card_width();
        card_height = shape->card_height();
        lean = shape->lean();
    }
    width = MAX(width, 0.01f);
    card_width = MAX(card_width, 0.01f);
    card_height = MAX(card_height, 0.01f);

    const float pixel_size = 0.05f;

    int W = (int)Math::floor(card_width / pixel_size);
    int H = (int)Math::floor(card_height / pixel_size);

    const float w_pixels = width / pixel_size;
    const float lean_pixels = lean / pixel_size;

    const Color bark_light(0.55f, 0.38f, 0.22f, 1.0f);
    const Color bark_dark(0.33f, 0.21f, 0.12f, 1.0f);
    Ref<Image> img = Image::create_empty(W, H, false, Image::FORMAT_RGBA8);

    for (int x = 0; x < W; ++x) {
        
        float t = (float)x / (float)W;

        float lean_progress = t;
        float cy = lean_pixels * lean_progress;

        float width_digress = 1.0f - t;
        float current_half_width = w_pixels * 0.5f * width_digress;

        int y_start = (int)Math::floor(cy - current_half_width);
        int y_end = (int)Math::ceil(cy + current_half_width);

        for (int y = y_start; y <= y_end; ++y) {
            if (y >= 0 && y < H) {
                float noise = Random::randf01(shape_id, y * W + x);
                Color pixel_color = bark_dark.lerp(bark_light, noise);
                img->set_pixel(x, y, pixel_color);
            }
        }
    }

    Ref<ImageTexture> texture = ImageTexture::create_from_image(img);

    Ref<StandardMaterial3D> material;
    material.instantiate();
    material->set_albedo(Color(1.0f, 1.0f, 1.0f, 1.0f));
    material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, texture);
    
    // Nearest с Mipmaps: сохраняет пиксельный вид вблизи, но не мерцает вдалеке
    material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS); 
    material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
    material->set_alpha_scissor_threshold(0.5f);
    
    // КРИТИЧЕСКИ ВАЖНО: Делает плоский квад видимым с обеих сторон
    material->set_cull_mode(BaseMaterial3D::CULL_DISABLED); 
    
    material->set_roughness(0.9f);

    return material;
}