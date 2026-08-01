#include "MeshMarker.hpp"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MeshMarker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &MeshMarker::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &MeshMarker::get_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0,100,0.01,or_greater"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("set_weight_multiplier", "weight"), &MeshMarker::set_weight_multiplier);
	ClassDB::bind_method(D_METHOD("get_weight_multiplier"), &MeshMarker::get_weight_multiplier);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "weight_multiplier", PROPERTY_HINT_RANGE, "0,10,0.01,or_greater"), "set_weight_multiplier", "get_weight_multiplier");
}

void MeshMarker::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Подписываемся на уведомление о смене локального transform-а,
			// чтобы сразу дёргать деформацию родителя при перемещении маркера
			// (и в редакторе, и из кода генератора).
			set_notify_local_transform(true);
		} break;

		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			Node *parent = get_parent();
			if (parent != nullptr && parent->has_method("rebuild_deformed_mesh")) {
				parent->call("rebuild_deformed_mesh");
			}
		} break;
	}
}

void MeshMarker::set_radius(real_t p_radius) {
	radius = p_radius;
}

real_t MeshMarker::get_radius() const {
	return radius;
}

void MeshMarker::set_weight_multiplier(real_t p_weight) {
	weight_multiplier = p_weight;
}

real_t MeshMarker::get_weight_multiplier() const {
	return weight_multiplier;
}