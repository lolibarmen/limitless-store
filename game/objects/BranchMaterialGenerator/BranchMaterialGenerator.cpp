#include "BranchMaterialGenerator.hpp"
#include <Utils/RandomUtils.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <cmath>

using namespace godot;

void BranchMaterialGenerator::_bind_methods() {}

Ref<Material> BranchMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    constexpr int W = 16;
    constexpr int H = 16;

    // Пиксель-арт ветки: 16x16 RGBA, прозрачный фон.
    // Верх изображения (UV.y=0) — основание ветки, низ (UV.y=1) — кончик.
    Ref<Image> img = Image::create_empty(W, H, false, Image::FORMAT_RGBA8);

    const Color bark_light(0.55f, 0.38f, 0.22f, 1.0f);
    const Color bark_dark(0.33f, 0.21f, 0.12f, 1.0f);
    const Color outline(0.20f, 0.12f, 0.07f, 1.0f);

    for (int y = 0; y < H; ++y) {
        const float t = (H > 1) ? (float)y / (float)(H - 1) : 0.0f;

        // Сужение к кончику + лёгкий изгиб.
        const float cx = (W - 1) * 0.5f + 2.0f * t * t;
        const float hw = 5.5f + (1.2f - 5.5f) * t;

        for (int x = 0; x < W; ++x) {
            const float d = Math::abs((float)x - cx);
            if (d > hw) continue;

            Color c;
            if (d > hw - 1.0f) {
                c = outline;                       // тёмный контур по краю силуэта
            } else {
                const float n = Random::randf01(shape_id, y * W + x);
                c = bark_dark.lerp(bark_light, n); // шум коры между двумя коричневыми
            }
            img->set_pixel(x, y, c);
        }
    }

    Ref<ImageTexture> texture = ImageTexture::create_from_image(img);

    Ref<StandardMaterial3D> material;
    material.instantiate();
    material->set_albedo(Color(1.0f, 1.0f, 1.0f, 1.0f));
    material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, texture);
    material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS);
    material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
    material->set_alpha_scissor_threshold(0.5f);
    material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
    material->set_roughness(0.9f);
    return material;
}
