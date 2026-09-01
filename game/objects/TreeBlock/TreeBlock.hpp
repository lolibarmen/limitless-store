#pragma once

#include <MeshPrimitive/MeshPrimitive.hpp>

namespace godot {

class TreeBlock : public MeshPrimitive {
	GDCLASS(TreeBlock, MeshPrimitive)

private:
	// Эти свойства нужны генератору для расчёта смещений, 
	// даже если меш и маркеры добавлены вручную.
	real_t radius = 1.0;
	real_t height = 2.0;

protected:
	static void _bind_methods();

public:
	TreeBlock() = default;
	~TreeBlock() override = default;

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_height(real_t p_height);
	real_t get_height() const;
};

} // namespace godot