# presets/campfire_dusk.gd
extends AtmospherePreset
func _init() -> void:
	# Небо — глубокий розово-пурпурный закат с пепельным горизонтом
	sky_top_color            = Color(0.102, 0.071, 0.118, 1.0)   # тёмно-фиолетовый зенит
	sky_horizon_color        = Color(0.612, 0.318, 0.310, 1.0)   # грязно-розовый горизонт
	sky_ground_horizon_color = Color(0.459, 0.243, 0.216, 1.0)   # пыльно-терракотовый
	sky_ground_bottom_color  = Color(0.118, 0.094, 0.102, 1.0)   # почти чёрный низ
	sky_sun_angle_max        = 8.0    # солнце почти за горизонтом
	sky_sun_curve            = 0.08

	# Ambient — холодный синевато-пепельный отсвет снега
	ambient_color            = Color(0.380, 0.298, 0.353, 1.0)   # лиловато-серый
	ambient_sky_contribution = 0.55
	ambient_energy           = 0.28   # сцена тёмная, костёр — главный источник света

	# Туман — плотный, тёплый у горизонта (дым + мороз)
	fog_enabled              = true
	fog_color                = Color(0.310, 0.200, 0.208, 1.0)   # пурпурно-серый туман
	fog_light_color          = Color(0.576, 0.298, 0.224, 1.0)   # тёплый рассеянный свет
	fog_sun_scatter          = 0.35   # сильный скаттеринг от низкого солнца
	fog_density              = 0.038
	fog_aerial_perspective   = 0.75   # горы тонут в тумане

	# Объёмный туман — дыхание мороза и дым костра
	volumetric_fog_enabled   = true
	volumetric_fog_density   = 0.018

	# Тонмаппинг — кинематографичный, немного пережжённый
	tonemap_mode             = 2      # Filmic
	tonemap_exposure         = 0.72   # немного недоэкспонировано — тёмная ночь
	tonemap_white            = 8.0    # высокий white point для контраста костра

	# Свечение — ореол костра, звёзды
	glow_enabled             = true
	glow_intensity           = 1.4    # заметное свечение от огня
	glow_bloom               = 0.22   # мягкий блум

	# SSAO — глубокие тени под снегом, между деревьями
	ssao_enabled             = true
	ssao_intensity           = 2.4

	# Коррекция — десатурация, высокий контраст, холод
	adjustment_brightness    = 0.82   # темнее общая сцена
	adjustment_contrast      = 1.18   # резкий контраст снег/тень
	adjustment_saturation    = 0.42   # почти монохромно, только костёр насыщен

	# Солнце — едва над горизонтом, тёплое но слабое
	sun_color                = Color(0.871, 0.482, 0.357, 1.0)   # оранжево-красный закат
	sun_energy               = 0.35   # очень слабое, почти зашло
	sun_angular_distance     = 1.8    # чуть шире — размытый диск в тумане
