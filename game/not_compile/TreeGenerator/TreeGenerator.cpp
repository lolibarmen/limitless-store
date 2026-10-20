#include "TreeGenerator.hpp"
#include <MeshCylinder/MeshCylinder.hpp>
#include <MeshYJoint/MeshYJoint.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/cylinder_mesh.hpp>

using namespace godot;

void TreeGenerator::_bind_methods() {
    // --- Свойства ствола ---
    ClassDB::bind_method(D_METHOD("set_block_scene", "scene"), &TreeGenerator::set_block_scene);
    ClassDB::bind_method(D_METHOD("get_block_scene"), &TreeGenerator::get_block_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "block_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_block_scene", "get_block_scene");

    ClassDB::bind_method(D_METHOD("set_num_blocks", "num"), &TreeGenerator::set_num_blocks);
    ClassDB::bind_method(D_METHOD("get_num_blocks"), &TreeGenerator::get_num_blocks);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "num_blocks", PROPERTY_HINT_RANGE, "1,50,1"), "set_num_blocks", "get_num_blocks");

    ClassDB::bind_method(D_METHOD("set_base_radius", "radius"), &TreeGenerator::set_base_radius);
    ClassDB::bind_method(D_METHOD("get_base_radius"), &TreeGenerator::get_base_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_radius", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_base_radius", "get_base_radius");

    ClassDB::bind_method(D_METHOD("set_block_height", "height"), &TreeGenerator::set_block_height);
    ClassDB::bind_method(D_METHOD("get_block_height"), &TreeGenerator::get_block_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "block_height", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_block_height", "get_block_height");

    ClassDB::bind_method(D_METHOD("set_taper", "taper"), &TreeGenerator::set_taper);
    ClassDB::bind_method(D_METHOD("get_taper"), &TreeGenerator::get_taper);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "taper", PROPERTY_HINT_RANGE, "0.0,2.0,0.01"), "set_taper", "get_taper");

    // --- Свойства Y-соединения ---
    ClassDB::bind_method(D_METHOD("set_joint_scene", "scene"), &TreeGenerator::set_joint_scene);
    ClassDB::bind_method(D_METHOD("get_joint_scene"), &TreeGenerator::get_joint_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "joint_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_joint_scene", "get_joint_scene");

    ClassDB::bind_method(D_METHOD("set_joint_block_index", "index"), &TreeGenerator::set_joint_block_index);
    ClassDB::bind_method(D_METHOD("get_joint_block_index"), &TreeGenerator::get_joint_block_index);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_block_index", PROPERTY_HINT_RANGE, "-1,50,1"), "set_joint_block_index", "get_joint_block_index");

    ClassDB::bind_method(D_METHOD("set_joint_spread", "spread"), &TreeGenerator::set_joint_spread);
    ClassDB::bind_method(D_METHOD("get_joint_spread"), &TreeGenerator::get_joint_spread);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "joint_spread", PROPERTY_HINT_RANGE, "0.1,3.0,0.1"), "set_joint_spread", "get_joint_spread");

    ClassDB::bind_method(D_METHOD("set_joint_junction_height", "height"), &TreeGenerator::set_joint_junction_height);
    ClassDB::bind_method(D_METHOD("get_joint_junction_height"), &TreeGenerator::get_joint_junction_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "joint_junction_height", PROPERTY_HINT_RANGE, "0.0,1.0,0.05"), "set_joint_junction_height", "get_joint_junction_height");

    ClassDB::bind_method(D_METHOD("set_joint_branch_radius_mult", "mult"), &TreeGenerator::set_joint_branch_radius_mult);
    ClassDB::bind_method(D_METHOD("get_joint_branch_radius_mult"), &TreeGenerator::get_joint_branch_radius_mult);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "joint_branch_radius_mult", PROPERTY_HINT_RANGE, "0.1,1.0,0.05"), "set_joint_branch_radius_mult", "get_joint_branch_radius_mult");

    ClassDB::bind_method(D_METHOD("generate"), &TreeGenerator::generate);
}

TreeGenerator::TreeGenerator() {
    // По умолчанию размещаем разветвление на самом верхнем блоке
    joint_block_index = 4; 
}

TreeGenerator::~TreeGenerator() {
}

void TreeGenerator::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        generate();
    }
}

// --- Геттеры и сеттеры (ствол) ---
void TreeGenerator::set_block_scene(const Ref<PackedScene> &p_scene) { if (block_scene != p_scene) { block_scene = p_scene; generate(); } }
Ref<PackedScene> TreeGenerator::get_block_scene() const { return block_scene; }

void TreeGenerator::set_num_blocks(int p_num) { if (p_num != num_blocks) { num_blocks = p_num; generate(); } }
int TreeGenerator::get_num_blocks() const { return num_blocks; }

void TreeGenerator::set_base_radius(real_t p_radius) { if (p_radius != base_radius) { base_radius = p_radius; generate(); } }
real_t TreeGenerator::get_base_radius() const { return base_radius; }

void TreeGenerator::set_block_height(real_t p_height) { if (p_height != block_height) { block_height = p_height; generate(); } }
real_t TreeGenerator::get_block_height() const { return block_height; }

void TreeGenerator::set_taper(real_t p_taper) { if (p_taper != taper) { taper = p_taper; generate(); } }
real_t TreeGenerator::get_taper() const { return taper; }

// --- Геттеры и сеттеры (Y-соединение) ---
void TreeGenerator::set_joint_scene(const Ref<PackedScene> &p_scene) { if (joint_scene != p_scene) { joint_scene = p_scene; generate(); } }
Ref<PackedScene> TreeGenerator::get_joint_scene() const { return joint_scene; }

void TreeGenerator::set_joint_block_index(int p_index) { 
    if (p_index != joint_block_index) { 
        joint_block_index = p_index; 
        generate(); 
    } 
}
int TreeGenerator::get_joint_block_index() const { return joint_block_index; }

void TreeGenerator::set_joint_spread(real_t p_spread) { if (p_spread != joint_spread) { joint_spread = p_spread; generate(); } }
real_t TreeGenerator::get_joint_spread() const { return joint_spread; }

void TreeGenerator::set_joint_junction_height(real_t p_height) { if (p_height != joint_junction_height) { joint_junction_height = p_height; generate(); } }
real_t TreeGenerator::get_joint_junction_height() const { return joint_junction_height; }

void TreeGenerator::set_joint_branch_radius_mult(real_t p_mult) { if (p_mult != joint_branch_radius_mult) { joint_branch_radius_mult = p_mult; generate(); } }
real_t TreeGenerator::get_joint_branch_radius_mult() const { return joint_branch_radius_mult; }


// =========================================================================
// ОСНОВНОЙ МЕТОД ГЕНЕРАЦИИ
// =========================================================================
void TreeGenerator::generate() {
    if (num_blocks < 1) {
        set_mesh(Ref<Mesh>());
        return;
    }

    Ref<ArrayMesh> final_trunk_mesh;
    final_trunk_mesh.instantiate();

    real_t current_radius = base_radius;

    for (int i = 0; i < num_blocks; ++i) {
        MeshPrimitive* current_primitive = nullptr;
        Vector3 segment_global_pos = Vector3(0, i * block_height, 0);

        // 1. Определяем, используем ли мы Y-соединение на этом шаге
        bool is_joint = (joint_scene.is_valid() && i == joint_block_index);

        if (is_joint) {
            MeshYJoint* y_joint = Object::cast_to<MeshYJoint>(joint_scene->instantiate());
            if (!y_joint) {
                y_joint = memnew(MeshYJoint);
                // Фоллбек для Y-joint, если нет сцены, можно оставить пустым или создать базовый меш
            }
            current_primitive = y_joint;

            if (current_primitive->get_marker_count() == 0) {
                current_primitive->setup_markers();
            }

            // Настраиваем параметры Y-соединения
            MeshYJoint* joint_ptr = Object::cast_to<MeshYJoint>(current_primitive);
            if (joint_ptr) {
                joint_ptr->set_base_radius(current_radius);
                joint_ptr->set_junction_radius(current_radius * 0.9); // Чуть уже основания
                joint_ptr->set_left_radius(current_radius * joint_branch_radius_mult);
                joint_ptr->set_right_radius(current_radius * joint_branch_radius_mult);
                joint_ptr->set_spread(joint_spread);
                joint_ptr->set_junction_height(joint_junction_height);
            }

        } else {
            // 2. Стандартный цилиндр
            MeshCylinder* cylinder = nullptr;
            if (block_scene.is_valid()) {
                cylinder = Object::cast_to<MeshCylinder>(block_scene->instantiate());
            }
            if (!cylinder) {
                cylinder = memnew(MeshCylinder);
                Ref<CylinderMesh> default_mesh;
                default_mesh.instantiate();
                default_mesh->set_radial_segments(16);
                default_mesh->set_rings(1);
                cylinder->set_base_mesh(default_mesh);
            }
            current_primitive = cylinder;

            if (current_primitive->get_marker_count() == 0) {
                current_primitive->setup_markers();
            }

            real_t r_bottom = current_radius;
            real_t r_top = MAX(0.05, current_radius - taper);

            auto bottom_markers = current_primitive->get_markers_by_group("bottom");
            auto top_markers = current_primitive->get_markers_by_group("top");

            for (auto* marker : bottom_markers) {
                real_t original_r = Vector2(marker->rest_position.x, marker->rest_position.z).length();
                real_t scale = (original_r > 0.001) ? (r_bottom / original_r) : 1.0;
                marker->current_pos = Vector3(marker->rest_position.x * scale, marker->rest_position.y, marker->rest_position.z * scale);
            }

            for (auto* marker : top_markers) {
                real_t original_r = Vector2(marker->rest_position.x, marker->rest_position.z).length();
                real_t scale = (original_r > 0.001) ? (r_top / original_r) : 1.0;
                marker->current_pos = Vector3(marker->rest_position.x * scale, marker->rest_position.y, marker->rest_position.z * scale);
            }

            current_radius = r_top; // Уменьшаем радиус для следующего шага
        }

        // 3. Получаем деформированные данные из примитива
        TypedArray<Array> deformed_surfaces = current_primitive->get_deformed_arrays();

        // 4. Переносим вершины и добавляем в итоговый меш
        for (int s = 0; s < deformed_surfaces.size(); s++) {
            Array surface_arrays = deformed_surfaces[s];
            
            if (surface_arrays.size() > Mesh::ARRAY_VERTEX && surface_arrays[Mesh::ARRAY_VERTEX].get_type() == Variant::PACKED_VECTOR3_ARRAY) {
                PackedVector3Array vertices = surface_arrays[Mesh::ARRAY_VERTEX];
                for (int v = 0; v < vertices.size(); v++) {
                    vertices[v] += segment_global_pos;
                }
                surface_arrays[Mesh::ARRAY_VERTEX] = vertices;
            }

            final_trunk_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface_arrays);
            
            // Копирование материала (приоритет: Surface Override -> Base Mesh Material)
            Ref<Material> mat = current_primitive->get_surface_override_material(s);
            if (mat.is_null() && current_primitive->get_base_mesh().is_valid()) {
                mat = current_primitive->get_base_mesh()->surface_get_material(s);
            }
            
            if (mat.is_valid()) {
                final_trunk_mesh->surface_set_material(final_trunk_mesh->get_surface_count() - 1, mat);
            }
        }

        // 5. Очистка временного объекта
        memdelete(current_primitive);
    }

    set_mesh(final_trunk_mesh);
}