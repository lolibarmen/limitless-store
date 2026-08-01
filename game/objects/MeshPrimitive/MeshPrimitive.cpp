#include "MeshPrimitive.hpp"

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace godot;

namespace {
// Вспомогательная структура: снимок влияния одного маркера на момент пересборки.
struct MarkerInfluence {
	Vector3 rest_position;
	Vector3 displacement;
	real_t radius;
	real_t weight_multiplier;
};
} // namespace

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

	ClassDB::bind_method(D_METHOD("bind_rest_pose"), &MeshPrimitive::bind_rest_pose);
	ClassDB::bind_method(D_METHOD("rebuild_deformed_mesh"), &MeshPrimitive::rebuild_deformed_mesh);

	ClassDB::bind_method(D_METHOD("get_marker", "name"), &MeshPrimitive::get_marker);
	ClassDB::bind_method(D_METHOD("get_all_markers"), &MeshPrimitive::get_all_markers);
	ClassDB::bind_method(D_METHOD("get_marker_names"), &MeshPrimitive::get_marker_names);
	ClassDB::bind_method(D_METHOD("get_marker_global_transform", "name"), &MeshPrimitive::get_marker_global_transform);
}

void MeshPrimitive::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Позиции маркеров, авторски расставленные разработчиком в
			// редакторе, фиксируются как точка покоя ровно один раз —
			// при первом входе в дерево (в том числе при каждом
			// инстанцировании сцены примитива генератором).
			bind_rest_pose();
		} break;
	}
}

void MeshPrimitive::_collect_rest_positions() {
	rest_positions.clear();
	int child_count = get_child_count();
	for (int i = 0; i < child_count; i++) {
		MeshMarker *marker = Object::cast_to<MeshMarker>(get_child(i));
		if (marker != nullptr) {
			rest_positions[marker->get_name()] = marker->get_position();
		}
	}
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

void MeshPrimitive::bind_rest_pose() {
	_collect_rest_positions();
	if (auto_rebuild) {
		rebuild_deformed_mesh();
	}
}

MeshMarker *MeshPrimitive::get_marker(const String &p_name) const {
	int child_count = get_child_count();
	for (int i = 0; i < child_count; i++) {
		MeshMarker *marker = Object::cast_to<MeshMarker>(get_child(i));
		if (marker != nullptr && marker->get_name() == p_name) {
			return marker;
		}
	}
	return nullptr;
}

TypedArray<MeshMarker> MeshPrimitive::get_all_markers() const {
	TypedArray<MeshMarker> result;
	int child_count = get_child_count();
	for (int i = 0; i < child_count; i++) {
		MeshMarker *marker = Object::cast_to<MeshMarker>(get_child(i));
		if (marker != nullptr) {
			result.push_back(marker);
		}
	}
	return result;
}

PackedStringArray MeshPrimitive::get_marker_names() const {
	PackedStringArray result;
	int child_count = get_child_count();
	for (int i = 0; i < child_count; i++) {
		MeshMarker *marker = Object::cast_to<MeshMarker>(get_child(i));
		if (marker != nullptr) {
			result.push_back(marker->get_name());
		}
	}
	return result;
}

Transform3D MeshPrimitive::get_marker_global_transform(const String &p_name) const {
	MeshMarker *marker = get_marker(p_name);
	if (marker != nullptr) {
		return marker->get_global_transform();
	}
	return get_global_transform();
}

void MeshPrimitive::rebuild_deformed_mesh() {
	if (base_mesh.is_null()) {
		return;
	}

	// Снимаем текущее смещение (displacement) каждого маркера относительно
	// его точки покоя (rest_position), зафиксированной в bind_rest_pose().
	std::vector<MarkerInfluence> influences;
	int child_count = get_child_count();
	for (int i = 0; i < child_count; i++) {
		MeshMarker *marker = Object::cast_to<MeshMarker>(get_child(i));
		if (marker == nullptr) {
			continue;
		}
		Vector3 rest = marker->get_position();
		if (rest_positions.has(marker->get_name())) {
			rest = rest_positions[marker->get_name()];
		}

		MarkerInfluence info;
		info.rest_position = rest;
		info.displacement = marker->get_position() - rest;
		info.radius = marker->get_radius();
		info.weight_multiplier = marker->get_weight_multiplier();
		influences.push_back(info);
	}

	Ref<ArrayMesh> result_mesh;
	result_mesh.instantiate();

	int surface_count = base_mesh->get_surface_count();
	for (int s = 0; s < surface_count; s++) {
		Array arrays = base_mesh->surface_get_arrays(s);
		if (arrays.is_empty()) {
			continue;
		}

		if (!influences.empty()) {
			PackedVector3Array vertices = arrays[Mesh::ARRAY_VERTEX];

			for (int v = 0; v < vertices.size(); v++) {
				Vector3 original = vertices[v];
				Vector3 total_displacement;
				real_t total_weight = 0.0;

				for (const MarkerInfluence &info : influences) {
					real_t distance = original.distance_to(info.rest_position);

					// Жёсткое отсечение по радиусу маркера, если он задан.
					if (info.radius > 0.0 && distance > info.radius) {
						continue;
					}

					real_t safe_distance = std::max<real_t>(distance, 0.0001);
					real_t weight = info.weight_multiplier / std::pow(safe_distance, falloff_power);

					total_displacement += info.displacement * weight;
					total_weight += weight;
				}

				if (total_weight > 0.0) {
					vertices[v] = original + total_displacement / total_weight;
				}
			}

			arrays[Mesh::ARRAY_VERTEX] = vertices;
		}

		// В godot-cpp Mesh::surface_get_primitive_type не публичен, поэтому
		// считаем, что base_mesh триангулирован (что верно для мешей,
		// экспортированных из Blender/большинства DCC-пакетов).
		result_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

		Ref<Material> material = base_mesh->surface_get_material(s);
		if (material.is_valid()) {
			result_mesh->surface_set_material(s, material);
		}
	}

	set_mesh(result_mesh);
}