#include "GravityAreaGenerator.hpp"
#include <GravityShape/GravityShape.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>

#include <cmath>
#include <vector>

using namespace godot;

namespace {
constexpr int kRings = 8;
constexpr int kSegments = 16;
} // namespace

void GravityAreaGenerator::_bind_methods() {}

void GravityAreaGenerator::generate(uint64_t shape_id, uint64_t chunk_id) const {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return;

    Ref<SemanticShape> base = sw->get_shape(shape_id);
    GravityShape* shape = Object::cast_to<GravityShape>(base.ptr());
    if (!shape) return;

    Vector3 origin;
    if (!get_chunk_origin(shape_id, chunk_id, origin)) return;

    MeshBuilder b = builder(shape_id, chunk_id, false);

    const float radius = Math::max(shape->get_area_radius(), 0.001f);
    std::vector<std::vector<Vertex>> rows(kRings + 1, std::vector<Vertex>(kSegments));

    for (int i = 0; i <= kRings; ++i) {
        const float theta = (float)Math_PI * (float)i / (float)kRings;
        const float y = std::cos(theta) * radius;
        const float ring_radius = std::sin(theta) * radius;

        for (int j = 0; j < kSegments; ++j) {
            const float phi = 2.0f * (float)Math_PI * (float)j / (float)kSegments;
            const Vector3 local(std::cos(phi) * ring_radius, y, std::sin(phi) * ring_radius);

            Vertex v;
            v.pos = local - origin;
            v.normal = local.normalized();
            v.uv = Vector2((float)j / (float)kSegments, (float)i / (float)kRings);
            rows[i][j] = v;
        }
    }

    for (int i = 0; i < kRings; ++i) {
        for (int j = 0; j < kSegments; ++j) {
            const int jn = (j + 1) % kSegments;
            const Vertex& a = rows[i][j];
            const Vertex& b0 = rows[i][jn];
            const Vertex& c = rows[i + 1][j];
            const Vertex& d = rows[i + 1][jn];

            if (i > 0) {
                b.emit(a, c, b0);
            }
            if (i + 1 < kRings) {
                b.emit(b0, c, d);
            }
        }
    }

    submit(shape_id, chunk_id, b);
}
