#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include <MeshMarker/MeshMarker.hpp>

namespace godot {

// Примитив-строительный блок с "клеточной" (cage) деформацией по маркерам.
//
// Маркеры — это дочерние узлы MeshMarker, которые разработчик просто
// расставляет в редакторе рядом с вершинами исходного меша (base_mesh),
// например по 8 углам куба. Их позиция в момент входа в сцену (_ready)
// фиксируется как "точка покоя". Любое дальнейшее смещение маркера —
// вручную в редакторе или из кода генератора — тянет за собой ближайшие
// к нему вершины base_mesh: вес влияния каждого маркера на вершину
// считается по обратному расстоянию от вершины до точки покоя маркера,
// нормализуется, и результат назначается в mesh (свойство MeshInstance3D).
//
// Для куба, у которого маркеры стоят точно в 8 вершинах, расстояние от
// "своей" вершины до маркера равно нулю, поэтому вес маркера доминирует
// и вершина просто следует за ним 1:1 — получается ожидаемое растяжение
// по углам. Для более плотных мешей соседние вершины будут плавно
// подтягиваться пропорционально близости к каждому маркеру.
class MeshPrimitive : public MeshInstance3D {
	GDCLASS(MeshPrimitive, MeshInstance3D)

	Ref<Mesh> base_mesh; // исходный, "недеформированный" меш
	real_t falloff_power = 2.0; // резкость затухания влияния маркера с расстоянием
	bool auto_rebuild = true; // пересобирать меш при биндинге и изменении маркеров

	Dictionary rest_positions; // String marker_name -> Vector3 (локальные координаты)

	void _collect_rest_positions();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	MeshPrimitive() = default;
	~MeshPrimitive() override = default;

	void set_base_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_base_mesh() const;

	void set_falloff_power(real_t p_power);
	real_t get_falloff_power() const;

	void set_auto_rebuild(bool p_enabled);
	bool get_auto_rebuild() const;

	// Зафиксировать текущее положение всех дочерних маркеров как их точку
	// покоя (нулевую деформацию). Вызывается автоматически при входе в
	// дерево сцены; можно вызвать и вручную, если нужно "запомнить" новую
	// недеформированную позу маркеров.
	void bind_rest_pose();

	MeshMarker *get_marker(const String &p_name) const;
	TypedArray<MeshMarker> get_all_markers() const;
	PackedStringArray get_marker_names() const;

	// Текущее (возможно, смещённое от точки покоя) глобальное положение
	// маркера — удобно для стыковки примитивов друг с другом в генераторе.
	Transform3D get_marker_global_transform(const String &p_name) const;

	// Пересчитать деформированный меш из base_mesh с учётом текущих
	// смещений маркеров и назначить результат в mesh. Вызывается
	// автоматически при перемещении любого дочернего MeshMarker.
	void rebuild_deformed_mesh();
};

} //namespace godot