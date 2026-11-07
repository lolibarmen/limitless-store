#include "TrunkMeshGenerator.hpp"
#include <TrunkShape/TrunkShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <Utils/RandomUtils.hpp>
#include <godot_cpp/core/math.hpp>
#include <cmath>
#include <vector>

using namespace godot;

void TrunkMeshGenerator::_bind_methods() {}

void TrunkMeshGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    auto* sw = SemanticWorld::get_singleton();
    if (!sw) return;
    Ref<SemanticShape> base = sw->get_shape(shape_id);
    TrunkShape* shape = Object::cast_to<TrunkShape>(base.ptr());
    if (!shape) return;

    Vector3 origin;
    if (!get_chunk_origin(shape_id, chunk_id, origin)) return;

    MeshBuilder b = builder(shape_id, chunk_id, false);

    const int R = Math::max(shape->get_radial_segments(), 3);
    const int H = Math::max(shape->get_height_segments(), 1);
    const float ring_var = shape->get_ring_variation();

    const Vector3 up = shape->get_up();
    const Vector3 helper = Math::abs(up.y) < 0.99f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    const Vector3 right = up.cross(helper).normalized();
    const Vector3 forward = right.cross(up).normalized();

    const float height = Math::max(shape->get_height(), 0.0001f);
    const float slope = (shape->get_top_radius() - shape->get_base_radius()) / height;

    std::vector<std::vector<Vertex>> rings(H + 1);
    for (int k = 0; k <= H; ++k) {
        const float t = (float)k / (float)H;
        const Vector3 c = shape->centerline(t) - origin;

        const float rv = 1.0f + (Random::randf01(shape_id, 100 + k) - 0.5f) * 2.0f * ring_var;
        const float r = shape->radius_at(t) * rv;

        auto& ring = rings[k];
        ring.reserve(R);
        for (int j = 0; j < R; ++j) {
            const float a = (float)j / (float)R * (float)Math_PI * 2.0f;
            const float cs = std::cos(a), sn = std::sin(a);

            Vertex v;
            v.pos = c + right * (cs * r) + forward * (sn * r);
            v.normal = (right * cs + forward * sn - up * slope).normalized();
            v.uv = Vector2((float)j / (float)R, t);
            ring.push_back(v);
        }
    }

    for (int i = 0; i + 1 <= H; ++i) {
        const auto& lo = rings[i];
        const auto& hi = rings[i + 1];
        for (int j = 0; j < R; ++j) {
            const int jn = (j + 1) % R;
            b.emit(lo[j], lo[jn], hi[j]);
            b.emit(lo[jn], hi[jn], hi[j]);
        }
    }

    {
        Vertex cap;
        cap.pos = shape->centerline(1.0f) - origin;
        cap.normal = up;
        cap.uv = Vector2(0.5f, 1.0f);

        const auto& top = rings[H];
        for (int j = 0; j < R; ++j) {
            const int jn = (j + 1) % R;
            b.emit(cap, top[j], top[jn]);
        }
    }

    submit(shape_id, chunk_id, b);
}
