#include "MeshPrimitive.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <algorithm>
#include <cmath>

using namespace godot;

void MeshPrimitive::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_base_mesh", "mesh"), &MeshPrimitive::set_base_mesh);
    ClassDB::bind_method(D_METHOD("get_base_mesh"), &MeshPrimitive::get_base_mesh);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "base_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_base_mesh", "get_base_mesh");

    ClassDB::bind_method(D_METHOD("set_falloff_power", "power"), &MeshPrimitive::set_falloff_power);
    ClassDB::bind_method(D_METHOD("get_falloff_power"), &MeshPrimitive::get_falloff_power);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "falloff_power", PROPERTY_HINT_RANGE, "0.1,8,0.05"), "set_falloff_power", "get_falloff_power");

    ClassDB::bind_method(D_METHOD("set_auto_rebuild", "enabled"), &MeshPrimitive::set_auto_rebuild);
    ClassDB::bind_method(D_METHOD("get_auto_rebuild"), &MeshPrimitive::get_auto_rebuild);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_rebuild"), "set_auto_rebuild", "get_auto_rebuild");

    // Привязка свойств отладки
    ClassDB::bind_method(D_METHOD("set_debug_markers_visible", "visible"), &MeshPrimitive::set_debug_markers_visible);
    ClassDB::bind_method(D_METHOD("is_debug_markers_visible"), &MeshPrimitive::is_debug_markers_visible);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_markers_visible"), "set_debug_markers_visible", "is_debug_markers_visible");

    ClassDB::bind_method(D_METHOD("rebuild_deformed_mesh"), &MeshPrimitive::rebuild_deformed_mesh);

    ClassDB::bind_method(D_METHOD("get_marker_count"), &MeshPrimitive::get_marker_count);
    ClassDB::bind_method(D_METHOD("get_marker_name", "index"), &MeshPrimitive::get_marker_name);
    ClassDB::bind_method(D_METHOD("get_marker_group", "index"), &MeshPrimitive::get_marker_group);
    ClassDB::bind_method(D_METHOD("get_marker_rest_position", "index"), &MeshPrimitive::get_marker_rest_position);
    ClassDB::bind_method(D_METHOD("get_marker_names"), &MeshPrimitive::get_marker_names);
    ClassDB::bind_method(D_METHOD("set_marker_position", "name", "new_pos"), &MeshPrimitive::set_marker_position);
}

void MeshPrimitive::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        setup_markers();
        if (auto_rebuild && is_inside_tree()) {
            rebuild_deformed_mesh();
        }
        // Инициализация дебажных маркеров при готовности, если они включены
        if (debug_markers_visible && is_inside_tree()) {
            _update_debug_markers();
        }
    }
}

void MeshPrimitive::setup_markers() {
    // Заглушка для переопределения в наследниках
}

void MeshPrimitive::add_marker(const String &p_group, const String &p_name, const Vector3 &p_pos, real_t p_radius, real_t p_weight) {
    MarkerData md;
    md.group = p_group;
    md.name = p_name;
    md.rest_position = p_pos;
    md.current_pos = p_pos;
    md.radius = p_radius;
    md.weight_multiplier = p_weight;
    markers.push_back(md);
    
    if (debug_markers_visible && is_inside_tree()) {
        _update_debug_markers();
    }
}

void MeshPrimitive::set_marker_position(const String &p_name, const Vector3 &p_new_pos) {
    for (auto &marker : markers) {
        if (marker.name == p_name) {
            marker.current_pos = p_new_pos;
            if (auto_rebuild && is_inside_tree()) {
                rebuild_deformed_mesh();
            }
            if (debug_markers_visible && is_inside_tree()) {
                _update_debug_markers();
            }
            return;
        }
    }
}

Vector3 MeshPrimitive::get_marker_position(const String &p_name) const {
    for (const auto &marker : markers) {
        if (marker.name == p_name) {
            return marker.current_pos;
        }
    }
    return Vector3();
}

std::vector<MarkerData*> MeshPrimitive::get_markers_by_group(const String &p_group) {
    std::vector<MarkerData*> result;
    for (auto &marker : markers) {
        if (marker.group == p_group) {
            result.push_back(&marker);
        }
    }
    return result;
}

void MeshPrimitive::rebuild_deformed_mesh() {
    if (base_mesh.is_null()) {
        return;
    }
    if (markers.empty()) {
        set_mesh(base_mesh);
        return;
    }

    Ref<ArrayMesh> result_mesh;
    result_mesh.instantiate();

    int surface_count = base_mesh->get_surface_count();
    for (int s = 0; s < surface_count; s++) {
        Array arrays = base_mesh->surface_get_arrays(s);
        if (arrays.is_empty()) {
            continue;
        }

        PackedVector3Array vertices = arrays[Mesh::ARRAY_VERTEX];
        
        struct Influence {
            Vector3 rest_pos;
            Vector3 displacement;
            real_t radius;
            real_t weight_mult;
        };
        
        std::vector<Influence> influences;
        influences.reserve(markers.size());
        
        for (size_t i = 0; i < markers.size(); i++) {
            const auto &m = markers[i];
            influences.push_back({
                m.rest_position,
                m.current_pos - m.rest_position,
                m.radius,
                m.weight_multiplier
            });
        }

        for (int v = 0; v < vertices.size(); v++) {
            Vector3 original = vertices[v];
            Vector3 total_displacement;
            real_t total_weight = 0.0;

            for (const auto &inf : influences) {
                real_t distance = original.distance_to(inf.rest_pos);

                if (inf.radius > 0.0 && distance > inf.radius) {
                    continue;
                }

                real_t safe_distance = std::max<real_t>(distance, 0.0001);
                real_t weight = inf.weight_mult / std::pow(safe_distance, falloff_power);

                total_displacement += inf.displacement * weight;
                total_weight += weight;
            }

            if (total_weight > 0.0) {
                vertices[v] = original + (total_displacement / total_weight);
            }
        }

        arrays[Mesh::ARRAY_VERTEX] = vertices;
        result_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        Ref<Material> material = base_mesh->surface_get_material(s);
        if (material.is_valid()) {
            result_mesh->surface_set_material(s, material);
        }
    }

    set_mesh(result_mesh);
}

void MeshPrimitive::set_base_mesh(const Ref<Mesh> &p_mesh) { 
    base_mesh = p_mesh; 
    if (auto_rebuild && is_inside_tree()) {
        rebuild_deformed_mesh(); 
    }
}

Ref<Mesh> MeshPrimitive::get_base_mesh() const { 
    return base_mesh; 
}

void MeshPrimitive::set_falloff_power(real_t p_power) { 
    falloff_power = p_power; 
    if (auto_rebuild && is_inside_tree()) {
        rebuild_deformed_mesh(); 
    }
}

real_t MeshPrimitive::get_falloff_power() const { 
    return falloff_power; 
}

void MeshPrimitive::set_auto_rebuild(bool p_enabled) { 
    auto_rebuild = p_enabled; 
}

bool MeshPrimitive::get_auto_rebuild() const { 
    return auto_rebuild; 
}

// --- Реализация методов отладки ---

void MeshPrimitive::set_debug_markers_visible(bool p_visible) {
    if (debug_markers_visible != p_visible) {
        debug_markers_visible = p_visible;
        if (is_inside_tree()) {
            _update_debug_markers();
        }
    }
}

bool MeshPrimitive::is_debug_markers_visible() const {
    return debug_markers_visible;
}

void MeshPrimitive::_update_debug_markers() {
    if (!is_inside_tree()) {
        return;
    }

    Node *container = get_node_or_null(NodePath("DebugMarkersContainer"));

    if (debug_markers_visible) {
        if (!container) {
            container = memnew(Node3D);
            container->set_name("DebugMarkersContainer");
            add_child(container);
        }

        std::vector<String> valid_names;
        valid_names.reserve(markers.size());

        for (const auto &marker : markers) {
            valid_names.push_back(marker.name);
            
            Node3D *debug_node = Object::cast_to<Node3D>(container->get_node_or_null(NodePath(marker.name)));
            if (!debug_node) {
                debug_node = memnew(MeshInstance3D);
                debug_node->set_name(marker.name);
                container->add_child(debug_node);
            }

            MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(debug_node);
            if (mesh_instance) {
                Ref<SphereMesh> sphere;
                sphere.instantiate();
                real_t radius = 0.05; //marker.radius > 0.0 ? marker.radius : 0.05;
                sphere->set_radius(radius);
                sphere->set_height(radius * 2.0);
                mesh_instance->set_mesh(sphere);

                Ref<StandardMaterial3D> material;
                material.instantiate();
                material->set_albedo(Color(1.0, 0.6, 0.0, 1.0)); // Ярко-оранжевый
                
                // ИСПРАВЛЕНИЕ ДЛЯ GODOT 4:
                material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
                material->set_depth_draw_mode(BaseMaterial3D::DEPTH_DRAW_DISABLED); // Рисуется поверх всех объектов
                
                mesh_instance->set_surface_override_material(0, material);
                mesh_instance->set_position(marker.current_pos);
            }
        }

        // Удаляем ноды, соответствующие удалённым маркерам
        int child_count = container->get_child_count();
        for (int i = child_count - 1; i >= 0; i--) {
            Node *child = container->get_child(i);
            String child_name = child->get_name();
            if (std::find(valid_names.begin(), valid_names.end(), child_name) == valid_names.end()) {
                container->remove_child(child);
                memdelete(child);
            }
        }
    } else {
        if (container) {
            remove_child(container);
            memdelete(container);
        }
    }
}

TypedArray<Array> MeshPrimitive::get_deformed_arrays() const {
    TypedArray<Array> result;
    
    if (base_mesh.is_null()) {
        return result;
    }

    int surface_count = base_mesh->get_surface_count();
    for (int s = 0; s < surface_count; s++) {
        Array arrays = base_mesh->surface_get_arrays(s);
        if (arrays.is_empty()) {
            continue;
        }

        // Если маркеров нет, просто возвращаем оригинальные массивы
        if (markers.empty()) {
            result.push_back(arrays);
            continue;
        }

        PackedVector3Array vertices = arrays[Mesh::ARRAY_VERTEX];
        
        struct Influence {
            Vector3 rest_pos;
            Vector3 displacement;
            real_t radius;
            real_t weight_mult;
        };
        
        std::vector<Influence> influences;
        influences.reserve(markers.size());
        
        for (size_t i = 0; i < markers.size(); i++) {
            const auto &m = markers[i];
            influences.push_back({
                m.rest_position,
                m.current_pos - m.rest_position,
                m.radius,
                m.weight_multiplier
            });
        }

        for (int v = 0; v < vertices.size(); v++) {
            Vector3 original = vertices[v];
            Vector3 total_displacement;
            real_t total_weight = 0.0;

            for (const auto &inf : influences) {
                real_t distance = original.distance_to(inf.rest_pos);

                if (inf.radius > 0.0 && distance > inf.radius) {
                    continue;
                }

                real_t safe_distance = std::max<real_t>(distance, 0.0001);
                real_t weight = inf.weight_mult / std::pow(safe_distance, falloff_power);

                total_displacement += inf.displacement * weight;
                total_weight += weight;
            }

            if (total_weight > 0.0) {
                vertices[v] = original + (total_displacement / total_weight);
            }
        }

        arrays[Mesh::ARRAY_VERTEX] = vertices;
        result.push_back(arrays);
    }

    return result;
}

// --- GDScript Getters ---

int MeshPrimitive::get_marker_count() const {
    return static_cast<int>(markers.size());
}

String MeshPrimitive::get_marker_name(int p_index) const {
    if (p_index >= 0 && p_index < static_cast<int>(markers.size())) {
        return markers[p_index].name;
    }
    return String();
}

String MeshPrimitive::get_marker_group(int p_index) const {
    if (p_index >= 0 && p_index < static_cast<int>(markers.size())) {
        return markers[p_index].group;
    }
    return String();
}

Vector3 MeshPrimitive::get_marker_rest_position(int p_index) const {
    if (p_index >= 0 && p_index < static_cast<int>(markers.size())) {
        return markers[p_index].rest_position;
    }
    return Vector3();
}

PackedStringArray MeshPrimitive::get_marker_names() const {
    PackedStringArray result;
    for (const auto &m : markers) {
        result.push_back(m.name);
    }
    return result;
}