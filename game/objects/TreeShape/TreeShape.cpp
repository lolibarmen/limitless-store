#include "TreeShape.hpp"
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <Utils/RandomUtils.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>

using namespace godot;

void TreeShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_origin", "origin"), &TreeShape::set_origin);
    ClassDB::bind_method(D_METHOD("get_origin"), &TreeShape::get_origin);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "origin"), "set_origin", "get_origin");

    ClassDB::bind_method(D_METHOD("set_up", "up"), &TreeShape::set_up);
    ClassDB::bind_method(D_METHOD("get_up"), &TreeShape::get_up);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "up"), "set_up", "get_up");

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &TreeShape::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &TreeShape::get_seed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

    ClassDB::bind_method(D_METHOD("set_height", "height"), &TreeShape::set_height);
    ClassDB::bind_method(D_METHOD("get_height"), &TreeShape::get_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.1,100.0,0.01"), "set_height", "get_height");

    ClassDB::bind_method(D_METHOD("set_base_radius", "radius"), &TreeShape::set_base_radius);
    ClassDB::bind_method(D_METHOD("get_base_radius"), &TreeShape::get_base_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_radius", PROPERTY_HINT_RANGE, "0.01,10.0,0.01"), "set_base_radius", "get_base_radius");

    ClassDB::bind_method(D_METHOD("set_top_radius", "radius"), &TreeShape::set_top_radius);
    ClassDB::bind_method(D_METHOD("get_top_radius"), &TreeShape::get_top_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "top_radius", PROPERTY_HINT_RANGE, "0.0,10.0,0.01"), "set_top_radius", "get_top_radius");

    ClassDB::bind_method(D_METHOD("set_bend", "bend"), &TreeShape::set_bend);
    ClassDB::bind_method(D_METHOD("get_bend"), &TreeShape::get_bend);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "bend"), "set_bend", "get_bend");

    ClassDB::bind_method(D_METHOD("set_radial_segments", "segments"), &TreeShape::set_radial_segments);
    ClassDB::bind_method(D_METHOD("get_radial_segments"), &TreeShape::get_radial_segments);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "radial_segments", PROPERTY_HINT_RANGE, "3,32,1"), "set_radial_segments", "get_radial_segments");

    ClassDB::bind_method(D_METHOD("set_height_segments", "segments"), &TreeShape::set_height_segments);
    ClassDB::bind_method(D_METHOD("get_height_segments"), &TreeShape::get_height_segments);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "height_segments", PROPERTY_HINT_RANGE, "1,64,1"), "set_height_segments", "get_height_segments");

    ClassDB::bind_method(D_METHOD("set_ring_variation", "variation"), &TreeShape::set_ring_variation);
    ClassDB::bind_method(D_METHOD("get_ring_variation"), &TreeShape::get_ring_variation);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ring_variation", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_ring_variation", "get_ring_variation");

    ClassDB::bind_method(D_METHOD("set_branch_count", "count"), &TreeShape::set_branch_count);
    ClassDB::bind_method(D_METHOD("get_branch_count"), &TreeShape::get_branch_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "branch_count", PROPERTY_HINT_RANGE, "0,32,1"), "set_branch_count", "get_branch_count");

    ClassDB::bind_method(D_METHOD("set_branch_start", "t"), &TreeShape::set_branch_start);
    ClassDB::bind_method(D_METHOD("get_branch_start"), &TreeShape::get_branch_start);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_start", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_branch_start", "get_branch_start");

    ClassDB::bind_method(D_METHOD("set_branch_end", "t"), &TreeShape::set_branch_end);
    ClassDB::bind_method(D_METHOD("get_branch_end"), &TreeShape::get_branch_end);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_end", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_branch_end", "get_branch_end");

    ClassDB::bind_method(D_METHOD("set_branch_length", "length"), &TreeShape::set_branch_length);
    ClassDB::bind_method(D_METHOD("get_branch_length"), &TreeShape::get_branch_length);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_length", PROPERTY_HINT_RANGE, "0.01,50.0,0.01"), "set_branch_length", "get_branch_length");

    ClassDB::bind_method(D_METHOD("set_branch_width", "width"), &TreeShape::set_branch_width);
    ClassDB::bind_method(D_METHOD("get_branch_width"), &TreeShape::get_branch_width);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_width", PROPERTY_HINT_RANGE, "0.01,10.0,0.01"), "set_branch_width", "get_branch_width");

    ClassDB::bind_method(D_METHOD("set_branch_spread", "spread"), &TreeShape::set_branch_spread);
    ClassDB::bind_method(D_METHOD("get_branch_spread"), &TreeShape::get_branch_spread);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "branch_spread", PROPERTY_HINT_RANGE, "0.0,1.57,0.01"), "set_branch_spread", "get_branch_spread");

    ClassDB::bind_method(D_METHOD("bind_planet", "planet"), &TreeShape::bind_planet);
    ClassDB::bind_method(D_METHOD("get_trunk_id"), &TreeShape::get_trunk_id);
}

void TreeShape::bind_planet(const Ref<PlanetShape>& planet) {
    _planet = planet;
    notify_shape_changed();
}

void TreeShape::set_up(const Vector3& v) {
    const float len = v.length();
    _up = len > 0.0001f ? v / len : Vector3(0.0f, 1.0f, 0.0f);
    notify_shape_changed();
}

void TreeShape::_spawn_children() {
    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw || _planet.is_null()) return;

    // --- Ствол ---
    Ref<TrunkShape> trunk;
    trunk.instantiate();
    trunk->bind_planet(_planet);
    trunk->set_origin(_origin);
    trunk->set_up(_up);
    trunk->set_height(_height);
    trunk->set_base_radius(_base_radius);
    trunk->set_top_radius(_top_radius);
    trunk->set_bend(_bend);
    trunk->set_radial_segments(_radial_segments);
    trunk->set_height_segments(_height_segments);
    trunk->set_ring_variation(_ring_variation);
    trunk->set_seed(_seed);
    sw->register_shape(trunk);
    _trunk_id = trunk->get_id();
    add_owned_shape(_trunk_id);

    // --- Ветки: позиции назначаются вдоль центральной линии ствола ---
    const int n = Math::max(_branch_count, 0);
    if (n == 0) return;

    // Золотой угол (~137.5°) для естественного филлотаксиса. 
    // Гарантирует максимально равномерное распределение веток по окружности без образования вертикальных рядов.
    const float golden_angle = Math_PI * (3.0f - Math::sqrt(5.0f)); 
    
    // Шаг по высоте для равномерного распределения без перекрытий
    const float height_range = _branch_end - _branch_start;
    const float step = height_range / (float)n;

    for (int i = 0; i < n; ++i) {
        // Параметр t вдоль ствола: центр бина + детерминированный джиттер.
        // Это предотвращает слипание веток по высоте и делает распределение естественным.
        const float jitter = (Random::randf01(_seed, 1000 + i) - 0.5f) * step * 0.8f;
        float t = _branch_start + (float)i * step + step * 0.5f + jitter;
        t = CLAMP(t, 0.05f, 0.98f);

        const Vector3 attach = trunk->centerline(t);
        const Vector3 tangent = trunk->centerline_tangent(t);

        // Ортонормированный базис вокруг касательной ствола.
        const Vector3 helper = Math::abs(tangent.y) < 0.99f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
        const Vector3 right = tangent.cross(helper).normalized();
        const Vector3 forward = right.cross(tangent).normalized();

        // Азимут: золотой угол + небольшой разброс для разрушения идеальной математической симметрии.
        float azimuth = (float)i * golden_angle;
        azimuth += (Random::randf01(_seed, 2000 + i) - 0.5f) * 0.6f; // ~34 градуса разброса
        
        const Vector3 radial = (right * std::cos(azimuth) + forward * std::sin(azimuth)).normalized();

        // Направление ветки: касательная ствола, отклонённая на угол spread к radial.
        // Апикальное доминирование: верхние ветки растут более вертикально (меньший угол разброса).
        const float height_factor = 1.0f - t * 0.4f; 
        const float spread = _branch_spread * height_factor * (0.8f + 0.4f * Random::randf01(_seed, 3000 + i));
        const Vector3 dir = (tangent * std::cos(spread) + radial * std::sin(spread)).normalized();

        // Чем выше по стволу, тем короче и тоньше ветка (естественное сужение кроны).
        const float length = _branch_length * (1.0f - 0.5f * t) * (0.8f + 0.1f * Random::randf01(_seed, 4000 + i));
        const float width = _branch_width * (1.0f - 0.5f * t) * (0.8f + 0.4f * Random::randf01(_seed, 5000 + i));

        Ref<BranchShape> branch;
        branch.instantiate();
        branch->bind_planet(_planet);
        branch->set_position(attach);
        branch->set_direction(dir);
        branch->set_length(Math::max(0.1f, length));
        branch->set_width(Math::max(0.05f, width));
        
        // Увеличенный множитель в seed предотвращает корреляцию случайных значений между разными параметрами ветки
        branch->set_seed(_seed + (uint64_t)i * 1000u + 1u); 
        
        sw->register_shape(branch);
        add_owned_shape(branch->get_id());
    }
}

void TreeShape::recompute_aabb() {
    SemanticWorld* sw = SemanticWorld::get_singleton();

    bool empty = true;
    Vector3 mn, mx;

    const PackedInt64Array owned = get_owned_shapes();
    for (int i = 0; i < owned.size(); ++i) {
        if (!sw) break;
        Ref<SemanticShape> child = sw->get_shape(owned[i]);
        if (child.is_null()) continue;

        const AABB a = child->get_aabb();
        const Vector3 cmin = a.position;
        const Vector3 cmax = a.position + a.size;

        if (empty) {
            mn = cmin; mx = cmax; empty = false;
        } else {
            mn = Vector3(Math::min(mn.x, cmin.x), Math::min(mn.y, cmin.y), Math::min(mn.z, cmin.z));
            mx = Vector3(Math::max(mx.x, cmax.x), Math::max(mx.y, cmax.y), Math::max(mx.z, cmax.z));
        }
    }

    if (empty) {
        // Дети ещё не созданы — вырожденный бокс в точке основания.
        mn = _origin;
        mx = _origin + Vector3(0.01f, 0.01f, 0.01f);
    }

    _aabb = AABB(mn, mx - mn);
    _aabb_dirty = false;
}

void TreeShape::on_register() {
    // Ни у дерева, ни у его детей нет собственного октодерева: всё подключено
    // к пространственному дереву планеты.
    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();
    if (mgr && _planet.is_valid()) {
        mgr->bind_spatially(get_id(), _planet->get_id());
    }

    if (_trunk_id != 0) {
        return; // дети уже созданы (повторная регистрация)
    }
    _spawn_children();
}
