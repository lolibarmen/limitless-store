#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

namespace godot {

class TreeGenerator : public Node3D {
	GDCLASS(TreeGenerator, Node3D)

private:
	Ref<PackedScene> block_scene; // Сцена с вашим вручную созданным TreeBlock, мешем и 16 маркерами
	
	int num_blocks = 5;
	real_t base_radius = 1.0;
	real_t block_height = 2.0;
	real_t taper = 0.1;      // Уменьшение радиуса на каждый блок
	real_t bend_x = 0.0;     // Накопительный изгиб по X
	real_t bend_z = 0.0;     // Накопительный изгиб по Z

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	TreeGenerator();
	~TreeGenerator() override = default;

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

	void set_bend_x(real_t p_bend);
	real_t get_bend_x() const;

	void set_bend_z(real_t p_bend);
	real_t get_bend_z() const;

	void generate();
};

} // namespace godot