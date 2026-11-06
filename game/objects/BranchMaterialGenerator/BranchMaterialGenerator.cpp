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
    // Геометрия карточки ветки: вертикальный прямоугольник, длинная ось вдоль
    // локального верха планеты. Высота — card_height, ширина — card_width
    // (width + запас на боковой сдвиг кончика). Текстура повторяет эти пропорции,
    // иначе пиксель-арт растягивается (квадратные пиксели).
    float width = 1.0f;
    float card_width = 1.0f;
    float card_height = 1.0f;
    float lean = 0.0f;
    if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
        Ref<SemanticShape> base = sw->get_shape(shape_id);
        BranchShape* shape = Object::cast_to<BranchShape>(base.ptr());
        if (shape) {
            width = shape->get_width();
            card_width = shape->card_width();
            card_height = shape->card_height();
            lean = shape->lean();
        }
    }
    if (width < 0.0001f) width = 0.0001f;
    if (card_width < 0.0001f) card_width = 0.0001f;
    if (card_height < 0.0001f) card_height = 0.0001f;

    const int W = 16;                              // текселей поперёк карточки
    const float texel = card_width / (float)W;     // мировой размер текселя (по ширине)
    int H = (int)Math::ceil(card_height / texel);  // текселей вдоль карточки
    H = CLAMP(H, 8, 256);

    // Масштаб V: ряд текстуры должен быть шириной ровно в один тексель.
    // uv1_scale.y умножает V, поэтому ряд занимает card_height / (H * texel) → texel.
    const float v_scale = card_height / ((float)H * texel);

    // Пиксель-арт ветки: W x H RGBA, прозрачный фон.
    // Верх изображения (UV.y=0) — основание ветки, низ — кончик.
    Ref<Image> img = Image::create_empty(W, H, false, Image::FORMAT_RGBA8);

    const Color bark_light(0.55f, 0.38f, 0.22f, 1.0f);
    const Color bark_dark(0.33f, 0.21f, 0.12f, 1.0f);
    const Color outline(0.20f, 0.12f, 0.07f, 1.0f);

    // Наклон ветки запекается в текстуру: основание полосы по центру карточки,
    // кончик смещён вбок ровно на lean (в текселях) — совпадает с проекцией ветки
    // на боковую ось, т.е. нарисованный кончик лёг на direction * length.
    const float base_cx = (float)(W - 1) * 0.5f;
    const float lean_texels = lean / texel;
    // Полуширина самой ветки в текселях (силуэт строится в долях от width, как в 16x16).
    const float w_texels = width / texel;

    for (int y = 0; y < H; ++y) {
        // Параметр вдоль карточки (V), 0 — основание, 1 — кончик.
        float t = (float)y * texel / card_height;
        t = CLAMP(t, 0.0f, 1.0f);

        // Сужение к кончику + лёгкий изгиб + наклон ветки, доли от ширины ветки.
        const float cx = base_cx + (2.0f / 16.0f) * w_texels * t * t + lean_texels * t;
        const float hw = ((5.5f + (1.2f - 5.5f) * t) / 16.0f) * w_texels;

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
    material->set_uv1_scale(Vector3(1.0f, v_scale, 1.0f));
    return material;
}