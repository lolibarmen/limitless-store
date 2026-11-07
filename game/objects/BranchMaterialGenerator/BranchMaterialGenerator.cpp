#include "BranchMaterialGenerator.hpp"
#include "godot_cpp/core/memory.hpp"
#include <BranchShape/BranchShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <Utils/RandomUtils.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <functional>

using namespace godot;

void BranchMaterialGenerator::_bind_methods() {}

Ref<Material> BranchMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    float root_width = 1.0f;
    float card_width = 1.0f;
    float card_height = 1.0f;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    Ref<SemanticShape> base = sw->get_shape(shape_id);
    Ref<BranchShape> shape = Object::cast_to<BranchShape>(base.ptr());
    if (shape.is_valid()) {
        root_width = shape->get_width();
        card_width = shape->card_width();
        card_height = shape->card_height();
    }
    root_width = MAX(root_width, 0.01f);
    card_width = MAX(card_width, 0.01f);
    card_height = MAX(card_height, 0.01f);

    const float pixel_size = 0.025f;

    int W = (int)Math::floor(card_width / pixel_size);
    int H = (int)Math::floor(card_height / pixel_size);

    const Color bark_light(0.55f, 0.38f, 0.22f, 1.0f);
    const Color bark_dark(0.33f, 0.21f, 0.12f, 1.0f);
    Ref<Image> img = Image::create_empty(W, H, false, Image::FORMAT_RGBA8);

    img->fill(Color(0, 0, 0, 0));

    RandomNumberGenerator* rng = memnew(RandomNumberGenerator);
    rng->randomize();

    auto draw_branch = [&](Vector2 a, Vector2 b, float width, Color color) {
        float radius = width * 0.5f;
    
        int min_x = MAX(0, (int)Math::floor(MIN(a.x, b.x) - radius));
        int max_x = MIN(W - 1, (int)Math::ceil(MAX(a.x, b.x) + radius));
        int min_y = MAX(0, (int)Math::floor(MIN(a.y, b.y) - radius));
        int max_y = MIN(H - 1, (int)Math::ceil(MAX(a.y, b.y) + radius));
    
        Vector2 ab = b - a;
        float len_sq = ab.length_squared();
    
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                Vector2 p(x + 0.5f, y + 0.5f);
                float t = len_sq > 0.0f
                    ? CLAMP((p - a).dot(ab) / len_sq, 0.0f, 1.0f)
                    : 0.0f;
    
                if (p.distance_to(a + ab * t) <= radius)
                    img->set_pixel(x, y, color);
            }
        }
    };
    
    std::function<void(Vector2, float, float, float, int)> branch;
    branch = [&](Vector2 pos, float angle, float length, float width, int depth) {
        if (depth <= 0 || length < 1.0f || width < 0.5f)
            return;

        Vector2 end = pos + Vector2(Math::sin(angle), -Math::cos(angle)) * length;

        Color color = bark_light.lerp(bark_dark, rng->randf_range(0.0f, 0.35f));

        draw_branch(pos, end, width, color);
        
        if (depth == 1)
            return;

        int branches = rng->randf() < 0.75f ? 2 : 1;

        for (int i = 0; i < branches; ++i) {
            float side = (i == 0) ? -1.0f : 1.0f;
            float new_angle = angle + side * rng->randf_range(0.3f, 0.75f)
                                     + rng->randf_range(-0.12f, 0.12f);

            float new_length = length * rng->randf_range(0.62f, 0.78f);
            float new_width = width * rng->randf_range(0.55f, 0.72f);

            branch(
                pos.lerp(end, rng->randf_range(0.55f, 0.85f)),
                new_angle,
                new_length,
                new_width,
                depth - 1
            );
        }
    };

    branch(
        Vector2(W * 0.03f, H * 0.03f),
        M_PI * 0.75f,
        Math::sqrt((W * 0.94f) * (W * 0.94f) +
                   (H * 0.94f) * (H * 0.94f)) * 0.5f,
        root_width / pixel_size,
        7
    );

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