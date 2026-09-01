#include "TreeBlock.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void TreeBlock::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &TreeBlock::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &TreeBlock::get_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("set_height", "height"), &TreeBlock::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &TreeBlock::get_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.1,20,0.1"), "set_height", "get_height");
}

void TreeBlock::set_radius(real_t p_radius) {
	radius = p_radius;
	// Мы больше не вызываем здесь пересборку, так как меш задаётся вручную.
}

real_t TreeBlock::get_radius() const {
	return radius;
}

void TreeBlock::set_height(real_t p_height) {
	height = p_height;
}

real_t TreeBlock::get_height() const {
	return height;
}