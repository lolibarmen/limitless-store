extends Camera3D
var flashlight: bool = true

func add_mat():
	#var block_source, pos = cast_ray()
	#if block_source == null: return
	pass

func _input(event):
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
			add_mat()
		if event.button_index == MOUSE_BUTTON_RIGHT and event.pressed:
			pass
	if event is InputEventKey:
		if event.physical_keycode == KEY_F and event.pressed:
			flashlight_action()

func cast_ray():
	var space = get_world_3d().direct_space_state
	var from = project_ray_origin(get_viewport().size / 2)
	var to = from + project_ray_normal(get_viewport().size / 2) * 1000

	var params = PhysicsRayQueryParameters3D.create(from, to)
	var result = space.intersect_ray(params)
	
	if result and result.collider.has_method("get_block_source"):
		return [result.collider.get_block_source(), result.position]
	else:
		return null

func flashlight_action():
	flashlight = !flashlight
	get_child(0).visible = flashlight
