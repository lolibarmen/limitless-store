extends Camera3D

func _process(delta):
	if Input.is_action_just_pressed("ui_accept"):
		cast_ray()

func cast_ray():
	var space = get_world_3d().direct_space_state
	var from = project_ray_origin(get_viewport().size/2)
	var to = from + project_ray_normal(get_viewport().size/2) * 1000
	
	var params = PhysicsRayQueryParameters3D.create(from, to)
	var result = space.intersect_ray(params)
	
	if result:
		print("Попадание в: ", result.collider.name)
		
		# Вызываем метод on_ray_hit у ChunkCollider
		if result.collider.has_method("on_ray_hit"):
			result.collider.on_ray_hit(result)
