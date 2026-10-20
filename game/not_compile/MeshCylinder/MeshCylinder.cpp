#include "MeshCylinder.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>

using namespace godot;

void MeshCylinder::setup_markers() {
    markers.clear();

    int segments = 8; 
    real_t default_radius = 1.0;
    real_t height_half = 1.0;

    for (int i = 0; i < segments; i++) {
        real_t angle = (real_t)i / segments * Math_TAU;
        real_t x = std::cos(angle) * default_radius;
        real_t z = std::sin(angle) * default_radius;

        String top_name = "top_" + String::num_int64(i);
        add_marker("top", top_name, Vector3(x, height_half, z), 1.5, 1.0);

        String bottom_name = "bottom_" + String::num_int64(i);
        add_marker("bottom", bottom_name, Vector3(x, -height_half, z), 1.5, 1.0);
    }
}

void MeshCylinder::set_top_radius(real_t p_new_radius) {
    // Получаем все маркеры верхней группы
    auto top_markers = get_markers_by_group("top");
    
    for (auto* marker : top_markers) {
        // Нормализуем текущую позицию по X и Z, чтобы сохранить угол, но изменить радиус
        Vector3 dir = marker->current_pos;
        dir.y = 0;
        if (dir.length() > 0.001) {
            dir = dir.normalized() * p_new_radius;
        } else {
            dir = Vector3(p_new_radius, 0, 0); // Fallback
        }
        
        // Сохраняем исходную высоту (Y)
        marker->current_pos = Vector3(dir.x, marker->rest_position.y, dir.z);
    }

    if (auto_rebuild && is_inside_tree()) {
        rebuild_deformed_mesh();
    }
}

void MeshCylinder::set_bottom_radius(real_t p_new_radius) {
    auto bottom_markers = get_markers_by_group("bottom");
    for (auto* marker : bottom_markers) {
        Vector3 dir = marker->current_pos;
        dir.y = 0;
        if (dir.length() > 0.001) {
            dir = dir.normalized() * p_new_radius;
        } else {
            dir = Vector3(p_new_radius, 0, 0);
        }
        marker->current_pos = Vector3(dir.x, marker->rest_position.y, dir.z);
    }

    if (auto_rebuild && is_inside_tree()) {
        rebuild_deformed_mesh();
    }
}