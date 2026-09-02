#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/material.hpp>
// --- НОВЫЕ INCLUDE ДЛЯ ДЕБАГА ---
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/variant/color.hpp>
// --------------------------------

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <vector>

namespace godot {

struct MarkerData {
    String name;
    String group;
    Vector3 rest_position;
    Vector3 current_pos;
    real_t radius;
    real_t weight_multiplier;
};

class MeshPrimitive : public MeshInstance3D {
    GDCLASS(MeshPrimitive, MeshInstance3D)

protected:
    Ref<Mesh> base_mesh;
    real_t falloff_power = 2.0;
    bool auto_rebuild = true;
    bool debug_markers_visible = false;

    std::vector<MarkerData> markers;
    
    // --- НОВОЕ: Контейнер для дебажных сфер ---
    Node3D *debug_container = nullptr;

    void _notification(int p_what);
    void _update_debug_markers();
    static void _bind_methods();

public:
    MeshPrimitive() = default;
    ~MeshPrimitive() override = default;

    virtual void setup_markers();

    // --- Properties ---
    void set_base_mesh(const Ref<Mesh> &p_mesh);
    Ref<Mesh> get_base_mesh() const;
    
    void set_falloff_power(real_t p_power);
    real_t get_falloff_power() const;
    
    void set_auto_rebuild(bool p_enabled);
    bool get_auto_rebuild() const;

    TypedArray<Array> get_deformed_arrays() const;

    void set_debug_markers_visible(bool p_visible);
    bool is_debug_markers_visible() const;

    // --- Marker Management ---
    void add_marker(const String &p_group, const String &p_name, const Vector3 &p_pos, real_t p_radius = 0.0, real_t p_weight = 1.0);
    void set_marker_position(const String &p_name, const Vector3 &p_new_pos);
    Vector3 get_marker_position(const String &p_name) const;
    std::vector<MarkerData*> get_markers_by_group(const String &p_group);

    // --- Core Logic ---
    void rebuild_deformed_mesh();

    // --- GDScript Getters ---
    int get_marker_count() const;
    String get_marker_name(int p_index) const;
    String get_marker_group(int p_index) const;
    Vector3 get_marker_rest_position(int p_index) const;
    PackedStringArray get_marker_names() const;
};

} // namespace godot