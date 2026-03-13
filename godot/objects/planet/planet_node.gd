extends PlanetNode
func _ready() -> void:
	planet_data = PlanetData.new()
	
	var height_noise = FastNoiseLite.new()
	height_noise.seed = randi()
	height_noise.noise_type = FastNoiseLite.TYPE_PERLIN
	height_noise.frequency = 0.1
	height_noise.fractal_octaves = 2
	
	var cave_noise = FastNoiseLite.new()
	cave_noise.seed = randi()
	cave_noise.noise_type = FastNoiseLite.TYPE_SIMPLEX
	cave_noise.frequency = 0.008
	cave_noise.fractal_octaves = 2
	
	var base_height = -16
	var height_range = 6
	var surface_blend = 4.0  # На сколько блоков вниз идёт переход 0.0 → 1.0
	
	for x in range(-64, 64):
		for z in range(-64, 64):
			var surface_y = base_height + int(height_noise.get_noise_2d(x, z) * height_range)
			
			for y in range(-32, 1):
				if y > surface_y:
					continue
				
				if cave_noise.get_noise_3d(x, y, z) > 0.3:
					continue
				
				# Глубина под поверхностью: 0 на поверхности, растёт вниз
				var depth = float(surface_y - y)
				var density = clamp(depth / surface_blend, 0.0, 1.0)
				
				get_planet_data().set_block(Vector3i(x, y, z), 1, density)
	
	update_chunks()
