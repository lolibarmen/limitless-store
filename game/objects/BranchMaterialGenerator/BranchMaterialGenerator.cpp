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

    // Защита от деления на ноль и схлопывания
    width = MAX(width, 0.01f);
    card_width = MAX(card_width, 0.01f);
    card_height = MAX(card_height, 0.01f);

    // Фиксированный размер одного пикселя в мировых единицах (масштаб детализации)
    const float fixed_texel_world_size = 0.05f; 

    // Размеры текстуры в пикселях (округление вниз)
    int W = (int)Math::floor(card_width / fixed_texel_world_size);
    int H = (int)Math::floor(card_height / fixed_texel_world_size);

    // Ограничиваем разумными пределами для производительности
    W = CLAMP(W, 4, 128);
    H = CLAMP(H, 8, 256);

    Ref<Image> img = Image::create_empty(W, H, false, Image::FORMAT_RGBA8);

    const Color bark_light(0.55f, 0.38f, 0.22f, 1.0f);
    const Color bark_dark(0.33f, 0.21f, 0.12f, 1.0f);

    const float base_cx = (float)(W - 1) * 0.5f;
    
    // Переводим мировые величины в тексели
    const float w_texels = width / fixed_texel_world_size;
    const float lean_texels = lean / fixed_texel_world_size;

    // Проходим по каждой строке текстуры (снизу вверх)
    for (int y = 0; y < H; ++y) {
        // Нормализованная высота: 0.0 внизу, 1.0 на самом кончике
        float t = (float)y / (float)H;

        // 1. ФОРМУЛА ИЗГИБА (Квадратичная)
        // При t=0 (основание) смещение равно 0. Ветка растет прямо из ствола.
        // При t=1 (кончик) смещение равно полному lean_texels.
        // Квадрат t*t обеспечивает плавный, органичный изгиб без резких заломов.
        float cx = base_cx + (lean_texels * t * t);

        // 2. ФОРМУЛА СУЖЕНИЯ (Tapering)
        // У основания (t=0) множитель равен 1.0 (полная ширина).
        // У кончика (t=1) множитель равен 0.15 (ветка сужается, но не исчезает в точку, что выглядит лучше).
        float taper_factor = 1.0f - (t * 0.85f);
        
        // Половина ширины ветки на текущей высоте Y
        float current_half_width = (w_texels * 0.5f) * taper_factor;

        // Границы закрашивания по X
        int x_start = (int)Math::floor(cx - current_half_width);
        int x_end = (int)Math::ceil(cx + current_half_width);

        // Закрашиваем пиксели внутри рассчитанной толщины
        for (int x = x_start; x <= x_end; ++x) {
            if (x >= 0 && x < W) {
                // Используем детерминированный шум на основе координат и ID формы для текстуры коры
                float noise = Random::randf01(shape_id, y * W + x);
                
                // Плавное смешивание темной и светлой коры
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