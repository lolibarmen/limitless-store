#include "MeshYJoint.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <cmath>

using namespace godot;

void MeshYJoint::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_base_radius", "radius"), &MeshYJoint::set_base_radius);
    ClassDB::bind_method(D_METHOD("set_junction_radius", "radius"), &MeshYJoint::set_junction_radius);
    ClassDB::bind_method(D_METHOD("set_left_radius", "radius"), &MeshYJoint::set_left_radius);
    ClassDB::bind_method(D_METHOD("set_right_radius", "radius"), &MeshYJoint::set_right_radius);
    ClassDB::bind_method(D_METHOD("set_spread", "factor"), &MeshYJoint::set_spread);
    ClassDB::bind_method(D_METHOD("set_junction_height", "normalized"), &MeshYJoint::set_junction_height);
}

// ---------------------------------------------------------------------------
// Инициализация маркеров
// ---------------------------------------------------------------------------
void MeshYJoint::setup_markers() {
    markers.clear();

    const int segments = 8;
    const real_t base_r = 1.0;
    const real_t junction_r = 1.0;
    const real_t branch_r = 1.0;

    const real_t y_base = -1.0;
    const real_t y_junction = 0.0;
    const real_t y_top = 1.0;

    const real_t branch_offset_x = 1.5;

    for (int i = 0; i < segments; i++) {
        real_t angle = (real_t)i / segments * Math_TAU;
        real_t cos_a = std::cos(angle);
        real_t sin_a = std::sin(angle);

        // --- base: нижнее кольцо ---
        add_marker("base",
                   "base_" + String::num_int64(i),
                   Vector3(cos_a * base_r, y_base, sin_a * base_r),
                   1.5, 1.0);

        // --- junction: среднее кольцо ---
        add_marker("junction",
                   "junction_" + String::num_int64(i),
                   Vector3(cos_a * junction_r, y_junction, sin_a * junction_r),
                   1.5, 1.0);

        // --- left: верхнее левое кольцо ---
        add_marker("left",
                   "left_" + String::num_int64(i),
                   Vector3(-branch_offset_x + cos_a * branch_r, y_top, sin_a * branch_r),
                   1.2, 1.0);

        // --- right: верхнее правое кольцо ---
        add_marker("right",
                   "right_" + String::num_int64(i),
                   Vector3(branch_offset_x + cos_a * branch_r, y_top, sin_a * branch_r),
                   1.2, 1.0);
    }
}

static void set_group_radius(MeshPrimitive *self,
                             std::vector<MarkerData *> &group,
                             real_t new_radius,
                             real_t center_x,
                             real_t center_z) {
    for (auto *m : group) {
        Vector3 dir(m->current_pos.x - center_x,
                    0.0,
                    m->current_pos.z - center_z);
        if (dir.length() > 0.001) {
            dir = dir.normalized() * new_radius;
        } else {
            dir = Vector3(new_radius, 0.0, 0.0);
        }
        m->current_pos.x = center_x + dir.x;
        m->current_pos.z = center_z + dir.z;
        // Y не меняем
    }
}

void MeshYJoint::set_base_radius(real_t p_radius) {
    auto g = get_markers_by_group("base");
    set_group_radius(this, g, p_radius, 0.0, 0.0);
    if (auto_rebuild && is_inside_tree()) rebuild_deformed_mesh();
}

void MeshYJoint::set_junction_radius(real_t p_radius) {
    auto g = get_markers_by_group("junction");
    set_group_radius(this, g, p_radius, 0.0, 0.0);
    if (auto_rebuild && is_inside_tree()) rebuild_deformed_mesh();
}

void MeshYJoint::set_left_radius(real_t p_radius) {
    // Центр левого кольца — текущее среднее X/Z маркеров группы
    auto g = get_markers_by_group("left");
    real_t cx = 0, cz = 0;
    for (auto *m : g) { cx += m->current_pos.x; cz += m->current_pos.z; }
    if (!g.empty()) { cx /= g.size(); cz /= g.size(); }
    set_group_radius(this, g, p_radius, cx, cz);
    if (auto_rebuild && is_inside_tree()) rebuild_deformed_mesh();
}

void MeshYJoint::set_right_radius(real_t p_radius) {
    auto g = get_markers_by_group("right");
    real_t cx = 0, cz = 0;
    for (auto *m : g) { cx += m->current_pos.x; cz += m->current_pos.z; }
    if (!g.empty()) { cx /= g.size(); cz /= g.size(); }
    set_group_radius(this, g, p_radius, cx, cz);
    if (auto_rebuild && is_inside_tree()) rebuild_deformed_mesh();
}

void MeshYJoint::set_spread(real_t p_factor) {
    // Дефолтное смещение веток = 0.7. Умножаем на factor.
    const real_t default_offset = 0.7;
    real_t target = default_offset * p_factor;

    auto left = get_markers_by_group("left");
    for (auto *m : left) {
        // Сдвигаем X маркера: текущий центр левого кольца был ~(-0.7)
        // Новый центр = -target. Сдвиг = (-target) - (-default_offset)
        m->current_pos.x += (-target) - (-default_offset);
    }

    auto right = get_markers_by_group("right");
    for (auto *m : right) {
        m->current_pos.x += (target) - (default_offset);
    }

    if (auto_rebuild && is_inside_tree()) rebuild_deformed_mesh();
}

void MeshYJoint::set_junction_height(real_t p_normalized) {
    // p_normalized: 0 = самый низ (y_base), 1 = самый верх (y_top)
    real_t y_base = -1.0;
    real_t y_top = 1.0;
    real_t new_y = y_base + (y_top - y_base) * CLAMP(p_normalized, 0.0, 1.0);

    auto g = get_markers_by_group("junction");
    for (auto *m : g) {
        m->current_pos.y = new_y;
    }

    if (auto_rebuild && is_inside_tree()) rebuild_deformed_mesh();
}