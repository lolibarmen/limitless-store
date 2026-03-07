extends PlanetNode


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	planet_data = PlanetData.new()

	for x in range(-10, 10):
		for y in range(-1, 0):
			for z in range(-10, 10):
				var index = Vector3i(x,y,z)
				get_planet_data().set_block(index, 1.0)
				
	spawn_initial_chunks()
