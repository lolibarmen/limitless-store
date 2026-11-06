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

    // Маленькая фигура: фильтр по боксу чанка выключен, меш эмитится целиком.
    MeshBuilder b = builder(shape_id, chunk_id, false);

    const int R = Math::max(shape->get_radial_segments(), 3);
    const int H = Math::max(shape->get_height_segments(), 1);
    const uint64_t seed = shape->get_seed();
    const float ring_var = shape->get_ring_variation();

    const Vector3 up = shape->get_up();
    const Vector3 helper = Math::abs(up.y) < 0.99f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    const Vector3 right = up.cross(helper).normalized();
    const Vector3 forward = right.cross(up).normalized();

    const float height = Math::max(shape->get_height(), 0.0001f);
    const float slope = (shape->get_top_radius() - shape->get_base_radius()) / height;

    // Кольца: pos в мировых координатах -> локальные координаты чанка (world - origin).
    std::vector<std::vector<Vertex>> rings(H + 1);
    for (int k = 0; k <= H; ++k) {
        const float t = (float)k / (float)H;
        const Vector3 c = shape->centerline(t) - origin;

        // Дегформации колец: вариация радиуса + овальность, детерминированно из seed.
        const float rv = 1.0f + (Random::randf01(seed, 100 + k) - 0.5f) * 2.0f * ring_var;
        const float ax = 1.0f + (Random::randf01(seed, 200 + k) - 0.5f) * ring_var;
        const float az = 1.0f + (Random::randf01(seed, 300 + k) - 0.5f) * ring_var;
        const float r = shape->radius_at(t) * rv;

        auto& ring = rings[k];
        ring.reserve(R);
        for (int j = 0; j < R; ++j) {
            const float a = (float)j / (float)R * (float)Math_PI * 2.0f;
            const float cs = std::cos(a), sn = std::sin(a);

            const Vector3 pos = c + right * (cs * r * ax) + forward * (sn * r * az);

            // Нормаль эллипса в плоскости кольца + наклон из-за конусности.
            Vector3 n = (right * (cs / ax) + forward * (sn / az)).normalized() - up * slope;
            n = n.normalized();

            Vertex v;
            v.pos = pos;
            v.normal = n;
            v.uv = Vector2((float)j / (float)R, t);
            ring.push_back(v);
        }
    }

    // Боковая поверхность между соседними кольцами.
    // Godot: лицевая грань — по часовой стрелке снаружи, поэтому порядок
    // вершин такой, чтобы (v1-v0)x(v2-v0) смотрел внутрь (против внешней нормали).
    for (int i = 0; i + 1 <= H; ++i) {
        const auto& lo = rings[i];
        const auto& hi = rings[i + 1];
        for (int j = 0; j < R; ++j) {
            const int jn = (j + 1) % R;
            b.emit(lo[j], lo[jn], hi[j]);
            b.emit(lo[jn], hi[jn], hi[j]);
        }
    }

    // Макушка — веер по последнему кольцу.
    // Порядок вершин такой, чтобы (v1-v0)x(v2-v0) смотрел вниз (против внешней
    // нормали up) — иначе веер оказывается задней гранью и отсекается.
    {
        const Vector3 cap_pos = shape->centerline(1.0f) - origin;
        Vertex cap;
        cap.pos = cap_pos;
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
