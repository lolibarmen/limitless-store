extends Node3D

func _ready():
	print("🚀 Starting MEGA test parametric generation...")
	await get_tree().process_frame
	
	var semantic_world = Engine.get_singleton("SemanticWorld")
	if not semantic_world:
		push_error("SemanticWorld not found!")
		return

	var curve = SemanticCurve.new()
	
	# Создаем вертикальный столб прямо в центре мира (0,0,0)
	# Радиус 12.0 гарантированно перекроет шаг сетки (step=4)
	curve.add_point(Vector3(0, -20, 0), Vector3(), Vector3(), 12.0)
	curve.add_point(Vector3(0,  20, 0), Vector3(), Vector3(), 12.0)

	# Свойство 0 = Material ID, Свойство 1 = Radius (для справки)
	curve.set_property(0, 1.0)
	curve.set_property(1, 12.0)

	var shape_id = semantic_world.register_shape(curve)
	print("✅ MEGA Shape registered with ID: ", shape_id)
	print("📦 Shape AABB: ", curve.get_aabb())
