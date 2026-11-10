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

void draw_segment(Image* image, Vector2 a, Vector2 b, float thickness_a, float thickness_b, const Color& color) {
    const float max_radius = MAX(thickness_a, thickness_b);
    const int min_x = MAX(0, (int)Math::floor(MIN(a.x, b.x) - max_radius));
    const int max_x = MIN(image->get_width() - 1, (int)Math::ceil(MAX(a.x, b.x) + max_radius));
    const int min_y = MAX(0, (int)Math::floor(MIN(a.y, b.y) - max_radius));
    const int max_y = MIN(image->get_height() - 1, (int)Math::ceil(MAX(a.y, b.y) + max_radius));

    const Vector2 ab = b - a;
    const float len_sq = ab.length_squared();

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const Vector2 p(x + 0.5f, y + 0.5f);
            const float t = len_sq > 0.0f ? CLAMP((p - a).dot(ab) / len_sq, 0.0f, 1.0f) : 0.0f;
            const float radius = Math::lerp(thickness_a, thickness_b, t) * 0.5f;
            if (p.distance_to(a + ab * t) <= radius) {
                image->set_pixel(x, y, color);
            }
        }
    }
}

void draw_branch(Image* image, RandomNumberGenerator* rng, Vector2 pos, float angle,
                 float length, float thickness, int depth, int total_depth, float bend) {
    if (depth <= 0 || length < 1.0f || thickness < 0.5f) {
        return;
    }

    const float progress = (float)(total_depth - depth) / (float)total_depth;
    const float tip_thickness = thickness * rng->randf_range(0.62f, 0.8f);
    const float seg_angle = angle + bend * rng->randf_range(0.5f, 1.5f) + rng->randf_range(-0.1f, 0.1f);
    const Vector2 end = pos + Vector2(Math::sin(seg_angle), -Math::cos(seg_angle)) * length;

    draw_segment(image, pos, end, thickness, tip_thickness, BARK_LIGHT.lerp(BARK_DARK, rng->randf_range(0.0f, 0.35f)));

    const int remaining = depth - 1;
    if (remaining <= 0) {
        return;
    }

    if (rng->randf() < Math::lerp(0.95f, 0.55f, progress)) {
        draw_branch(image, rng, pos.lerp(end, rng->randf_range(0.85f, 1.0f)), seg_angle,
                    length * rng->randf_range(0.68f, 0.82f), tip_thickness, remaining, total_depth, bend);
    }

    if (rng->randf() < 0.8f * (1.0f - progress)) {
        const float side = rng->randf() < 0.5f ? -1.0f : 1.0f;
        const float child_angle = seg_angle + side * rng->randf_range(0.5f, 1.1f) + rng->randf_range(-0.15f, 0.15f);
        draw_branch(image, rng, pos.lerp(end, rng->randf_range(0.3f, 0.95f)), child_angle,
                    length * rng->randf_range(0.42f, 0.68f), tip_thickness * rng->randf_range(0.35f, 0.55f),
                    remaining, total_depth, bend);

        if (rng->randf() < 0.25f * (1.0f - progress)) {
            const float other_angle = seg_angle - side * rng->randf_range(0.5f, 1.1f) + rng->randf_range(-0.15f, 0.15f);
            draw_branch(image, rng, pos.lerp(end, rng->randf_range(0.25f, 0.85f)), other_angle,
                        length * rng->randf_range(0.42f, 0.68f), tip_thickness * rng->randf_range(0.35f, 0.55f),
                        remaining, total_depth, bend);
        }
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
    float draw_angle = Math_PI * 0.75f;

    if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
        Ref<SemanticShape> base = sw->get_shape(shape_id);
        Ref<BranchShape> shape = Object::cast_to<BranchShape>(base.ptr());
        if (shape.is_valid()) {
            root_width = shape->get_width();
            card_width = shape->card_width();
            card_height = shape->card_height();

            const float tilt = Math::atan2(
                shape->get_direction().dot(shape->side()),
                shape->get_direction().dot(shape->up())
            );
            draw_angle = Math_PI - tilt;
        }
    }

    root_width = MAX(root_width, 0.01f);
    card_width = MAX(card_width, 0.01f);
    card_height = MAX(card_height, 0.01f);

    const int image_width = MAX(1, (int)Math::floor(card_width / _pixel_size));
    const int image_height = MAX(1, (int)Math::floor(card_height / _pixel_size));

    Ref<Image> image = Image::create_empty(image_width, image_height, false, Image::FORMAT_RGBA8);
    image->fill(Color(0, 0, 0, 0));

    uint64_t seed = shape_id;
    seed ^= seed >> 30;
    seed *= 0xbf58476d1ce4e5b9ULL;
    seed ^= seed >> 27;
    seed *= 0x94d049bb133111ebULL;
    seed ^= seed >> 31;

    Ref<RandomNumberGenerator> rng;
    rng.instantiate();
    rng->set_seed(seed);
    const float bend = rng->randf_range(-0.15f, 0.15f);

    const float span_x = image_width * 0.94f;
    const float span_y = image_height * 0.94f;
    const float reach = Math::sqrt(span_x * span_x + span_y * span_y) * 0.5f;

    const float chain = (1.0f - Math::pow(0.75f, (float)_max_depth)) / 0.25f;

    draw_branch(image.ptr(), rng.ptr(), Vector2(image_width * 0.03f, image_height * 0.03f),
                draw_angle, reach / chain, root_width / _pixel_size, _max_depth, _max_depth, bend);

    return make_material(image);
}
