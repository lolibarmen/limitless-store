#pragma once

#include <godot_cpp/classes/node3d.hpp>

namespace godot {

// Маркер — точка управления "клеточной" (cage) деформацией примитива.
// Сам по себе ничего не считает — это просто узел-метка, который
// разработчик перетаскивает в редакторе рядом с нужными вершинами меша,
// как дочерний элемент MeshPrimitive. Всю логику (точка покоя, вес влияния
// на вершины) применяет сам MeshPrimitive, обходя своих детей.
//
// radius  — если > 0, маркер влияет только на вершины в этом радиусе от
//           точки покоя (жёсткое отсечение). 0 — влияние не ограничено,
//           только через затухание по расстоянию (falloff_power в MeshPrimitive).
// weight_multiplier — множитель "силы" маркера относительно остальных,
//           если один маркер должен доминировать/уступать другим.
class MeshMarker : public Node3D {
	GDCLASS(MeshMarker, Node3D)

	real_t radius = 0.0;
	real_t weight_multiplier = 1.0;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	MeshMarker() = default;
	~MeshMarker() override = default;

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_weight_multiplier(real_t p_weight);
	real_t get_weight_multiplier() const;
};

} //namespace godot