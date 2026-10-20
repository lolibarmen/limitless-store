#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

namespace godot {

class TreeGenerator : public MeshInstance3D {
    GDCLASS(TreeGenerator, MeshInstance3D)

private:
    // Сцена основного сегмента ствола
    Ref<PackedScene> block_scene;
    int num_blocks = 5;
    real_t base_radius = 1.0;
    real_t block_height = 2.0;
    real_t taper = 0.1;

    // Сцена и параметры Y-соединения (разветвления)
    Ref<PackedScene> joint_scene;
    int joint_block_index = -1; // -1 = не использовать, >= 0 = индекс блока для замены на Y-joint
    real_t joint_spread = 1.0;
    real_t joint_junction_height = 0.5;
    real_t joint_branch_radius_mult = 0.7; // Множитель радиуса веток относительно основания

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    TreeGenerator();
    ~TreeGenerator();

    void generate();

    // --- Свойства ствола ---
    void set_block_scene(const Ref<PackedScene> &p_scene);
    Ref<PackedScene> get_block_scene() const;

    void set_num_blocks(int p_num);
    int get_num_blocks() const;

    void set_base_radius(real_t p_radius);
    real_t get_base_radius() const;

    void set_block_height(real_t p_height);
    real_t get_block_height() const;

    void set_taper(real_t p_taper);
    real_t get_taper() const;

    // --- Свойства Y-соединения ---
    void set_joint_scene(const Ref<PackedScene> &p_scene);
    Ref<PackedScene> get_joint_scene() const;

    void set_joint_block_index(int p_index);
    int get_joint_block_index() const;

    void set_joint_spread(real_t p_spread);
    real_t get_joint_spread() const;

    void set_joint_junction_height(real_t p_height);
    real_t get_joint_junction_height() const;

    void set_joint_branch_radius_mult(real_t p_mult);
    real_t get_joint_branch_radius_mult() const;
};

}