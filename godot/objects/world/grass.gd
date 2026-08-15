extends Node3D

@export var grass_mesh: Mesh
@export var count := 5000
@export var area_size := Vector2(40, 40)  # размер области по XZ вокруг узла
@export var ray_height := 20.0            # высота старта луча над узлом
@export var ray_depth := 40.0             # максимальная длина луча вниз
@export var max_slope_deg := 45.0         # не ставить траву на крутых склонах
@export var collision_mask := 1           # какие слои "ловит" луч
@export var ground_offset := 0.02         # отступ по нормали, чтобы трава не тонула
@export var visibility_distance := 25.0   # дальность отрисовки травы

func _ready() -> void:
	var space := get_world_3d().direct_space_state
	var slope_threshold := cos(deg_to_rad(max_slope_deg))

	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.use_custom_data = true
	mm.mesh = grass_mesh
	mm.instance_count = count

	var placed := 0
	var attempts := 0
	var max_attempts := count * 10

	while placed < count and attempts < max_attempts:
		attempts += 1

		# Случайная точка над областью вокруг позиции этого узла
		var origin := global_transform.origin + Vector3(
			randf_range(-area_size.x * 0.5, area_size.x * 0.5),
			ray_height,
			randf_range(-area_size.y * 0.5, area_size.y * 0.5)
		)

		# Луч бьёт до ПЕРВОГО пересечения с любым коллайдером из маски
		var query := PhysicsRayQueryParameters3D.create(
			origin,
			origin + Vector3.DOWN * ray_depth,
			collision_mask
		)
		var result := space.intersect_ray(query)

		if result.is_empty():
			continue

		# Пропускаем слишком крутые поверхности
		if result.normal.dot(Vector3.UP) < slope_threshold:
			continue

		# Базис по нормали поверхности
		var n: Vector3 = result.normal
		var reference := Vector3.FORWARD if abs(n.dot(Vector3.FORWARD)) < 0.99 else Vector3.RIGHT
		var x_axis := n.cross(reference).normalized()
		var z_axis := x_axis.cross(n)
		var basis := Basis(x_axis, n, z_axis)

		# Случайный поворот вокруг нормали и масштаб
		var s := randf_range(0.8, 1.3)
		basis = basis.rotated(n, randf() * TAU).scaled(Vector3(s, s, s))

		var pos: Vector3 = result.position + n * ground_offset

		mm.set_instance_transform(placed, Transform3D(basis, pos))
		mm.set_instance_custom_data(placed, Color(randf(), randf(), 0.0, 0.0))
		placed += 1

	# Если часть лучей ни во что не попала — уменьшаем реальное число
	mm.instance_count = placed

	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	mmi.visibility_range_end = visibility_distance
	add_child(mmi)
