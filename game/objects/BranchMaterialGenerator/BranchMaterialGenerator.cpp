#include "BranchMaterialGenerator.hpp"
#include <BranchShape/BranchShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

namespace {

const Color BARK_LIGHT(0.55f, 0.38f, 0.22f, 1.0f);
const Color BARK_DARK(0.33f, 0.21f, 0.12f, 1.0f);

void draw_segment(Image* image, Vector2 a, Vector2 b, float thickness, const Color& color) {
    const float radius = thickness * 0.5f;
    const int min_x = MAX(0, (int)Math::floor(MIN(a.x, b.x) - radius));
    const int max_x = MIN(image->get_width() - 1, (int)Math::ceil(MAX(a.x, b.x) + radius));
    const int min_y = MAX(0, (int)Math::floor(MIN(a.y, b.y) - radius));
    const int max_y = MIN(image->get_height() - 1, (int)Math::ceil(MAX(a.y, b.y) + radius));

    const Vector2 ab = b - a;
    const float len_sq = ab.length_squared();

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const Vector2 p(x + 0.5f, y + 0.5f);
            const float t = len_sq > 0.0f ? CLAMP((p - a).dot(ab) / len_sq, 0.0f, 1.0f) : 0.0f;
            if (p.distance_to(a + ab * t) <= radius) {
                image->set_pixel(x, y, color);
            }
        }
    }
}

void draw_branch(Image* image, RandomNumberGenerator* rng, Vector2 pos, float angle, float length, float thickness, int depth) {
    if (depth <= 0 || length < 1.0f || thickness < 0.5f) {
        return;
    }

    const Vector2 end = pos + Vector2(Math::sin(angle), -Math::cos(angle)) * length;
    draw_segment(image, pos, end, thickness, BARK_LIGHT.lerp(BARK_DARK, rng->randf_range(0.0f, 0.35f)));

    if (depth == 1) {
        return;
    }

    const int count = rng->randf() < 0.75f ? 2 : 1;
    for (int i = 0; i < count; ++i) {
        const float side = i == 0 ? -1.0f : 1.0f;
        const float child_angle = angle + side * rng->randf_range(0.3f, 0.75f) + rng->randf_range(-0.12f, 0.12f);
        draw_branch(
            image,
            rng,
            pos.lerp(end, rng->randf_range(0.55f, 0.85f)),
            child_angle,
            length * rng->randf_range(0.62f, 0.78f),
            thickness * rng->randf_range(0.55f, 0.72f),
            depth - 1
        );
    }
}

Ref<Material> make_material(const Ref<Image>& image) {
    Ref<StandardMaterial3D> material;
    material.instantiate();
    material->set_albedo(Color(1.0f, 1.0f, 1.0f, 1.0f));
    material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, ImageTexture::create_from_image(image));
    material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS);
    material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
    material->set_alpha_scissor_threshold(0.5f);
    material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
    material->set_roughness(0.9f);
    return material;
}

} // namespace

void BranchMaterialGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_pixel_size", "size"), &BranchMaterialGenerator::set_pixel_size);
    ClassDB::bind_method(D_METHOD("get_pixel_size"), &BranchMaterialGenerator::get_pixel_size);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pixel_size", PROPERTY_HINT_RANGE, "0.001,0.5,0.001"), "set_pixel_size", "get_pixel_size");

    ClassDB::bind_method(D_METHOD("set_max_depth", "depth"), &BranchMaterialGenerator::set_max_depth);
    ClassDB::bind_method(D_METHOD("get_max_depth"), &BranchMaterialGenerator::get_max_depth);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_depth", PROPERTY_HINT_RANGE, "1,16,1"), "set_max_depth", "get_max_depth");
}

Ref<Material> BranchMaterialGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    float root_width = 1.0f;
    float card_width = 1.0f;
    float card_height = 1.0f;

    if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
        Ref<SemanticShape> base = sw->get_shape(shape_id);
        Ref<BranchShape> shape = Object::cast_to<BranchShape>(base.ptr());
        if (shape.is_valid()) {
            root_width = shape->get_width();
            card_width = shape->card_width();
            card_height = shape->card_height();
        }
    }

    root_width = MAX(root_width, 0.01f);
    card_width = MAX(card_width, 0.01f);
    card_height = MAX(card_height, 0.01f);

    const int image_width = MAX(1, (int)Math::floor(card_width / _pixel_size));
    const int image_height = MAX(1, (int)Math::floor(card_height / _pixel_size));

    Ref<Image> image = Image::create_empty(image_width, image_height, false, Image::FORMAT_RGBA8);
    image->fill(Color(0, 0, 0, 0));

    Ref<RandomNumberGenerator> rng;
    rng.instantiate();
    rng->randomize();

    const float span_x = image_width * 0.94f;
    const float span_y = image_height * 0.94f;
    const float reach = Math::sqrt(span_x * span_x + span_y * span_y) * 0.5f;

    draw_branch(image.ptr(), rng.ptr(), Vector2(image_width * 0.03f, image_height * 0.03f),
                Math_PI * 0.75f, reach, root_width / _pixel_size, _max_depth);

    return make_material(image);
}
