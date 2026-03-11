extends CharacterBody3D

# Параметры движения
@export var speed: float = 2.0
@export var jump_velocity: float = 4.5
@export var mouse_sensitivity: float = 0.1

# Внутренние переменные
var camera: Camera3D

# Get the gravity from the project settings to be synced with RigidBody nodes
var gravity: float = ProjectSettings.get_setting("physics/3d/default_gravity")

func _ready():
	# Создаем и настраиваем камеру
	camera = get_child(0)
	
	# Захватываем мышь
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _input(event):
	# Обработка движения мыши
	if event is InputEventMouseMotion and Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
		# Горизонтальное вращение (вокруг оси Y)
		rotate_y(deg_to_rad(-event.relative.x * mouse_sensitivity))
		
		# Вертикальное вращение (вокруг оси X камеры)
		camera.rotate_x(deg_to_rad(-event.relative.y * mouse_sensitivity))
		# Ограничиваем вертикальный угол, чтобы не перевернуться
		camera.rotation.x = clamp(camera.rotation.x, deg_to_rad(-89), deg_to_rad(89))
	
	# Освобождение/захват мыши по нажатию Escape
	if event.is_action_pressed("ui_cancel"):
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _physics_process(delta):
	# Add the gravity
	if not is_on_floor():
		velocity.y -= gravity * delta
	
	# Handle Jump (используем KEY_SPACE)
	if Input.is_key_pressed(KEY_SPACE):
		velocity.y = jump_velocity
	
	# Получаем вектор движения напрямую с клавиатуры
	var input_dir = Vector2.ZERO
	
	# WASD управление напрямую через is_key_pressed
	if Input.is_key_pressed(KEY_W):
		input_dir.y += 1  # вперед
	if Input.is_key_pressed(KEY_S):
		input_dir.y -= 1  # назад
	if Input.is_key_pressed(KEY_A):
		input_dir.x -= 1  # влево
	if Input.is_key_pressed(KEY_D):
		input_dir.x += 1  # вправо
	
	# Нормализуем вектор, если он не нулевой
	if input_dir.length() > 0:
		input_dir = input_dir.normalized()
	
	var direction = Vector3.ZERO
	
	# Преобразуем ввод в направление относительно камеры
	if input_dir.length() > 0:
		# Получаем базис камеры и убираем наклон
		var camera_basis = camera.global_transform.basis
		camera_basis = camera_basis.orthonormalized()
		
		# Движение вперед/назад (ось Z)
		direction += -camera_basis.z * input_dir.y
		# Движение влево/вправо (ось X)
		direction += camera_basis.x * input_dir.x
		
		# Движение по горизонтали (игнорируем вертикальную компоненту)
		direction.y = 0
		direction = direction.normalized()
	
	# Применяем движение (горизонтальное)
	var horizontal_velocity = direction * speed
	velocity.x = horizontal_velocity.x
	velocity.z = horizontal_velocity.z
	
	move_and_slide()
