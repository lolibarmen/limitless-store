extends Camera3D
var flashlight: bool = true

func _input(event):
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
			var result = cast_ray()
			if result:
				var pos = result.collider.get_position()
				result.collider.set_position(pos + Vector3(0.0, 1.0, 0.0))
		if event.button_index == MOUSE_BUTTON_RIGHT and event.pressed:
			var result = cast_ray()
			if result:
				result.collider.queue_free()
	if event is InputEventKey:
		if event.physical_keycode == KEY_F and event.pressed:
			flashlight_action()

func cast_ray():
	var space = get_world_3d().direct_space_state
	var from = project_ray_origin(get_viewport().size / 2)
	var to = from + project_ray_normal(get_viewport().size / 2) * 1000

	var params = PhysicsRayQueryParameters3D.create(from, to)
	var result = space.intersect_ray(params)
	
	#print(result.collider)
	#result.collider.queue_free()
	
	return result
	
	if result and result.collider.has_method("on_ray_hit"):
		return result

func flashlight_action():
	flashlight = !flashlight
	get_child(0).visible = flashlight
