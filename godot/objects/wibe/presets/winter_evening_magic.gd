# presets/winter_evening_magic.gd
extends AtmospherePreset
func _init() -> void:
	# Небо — глубокий сине-фиолетовый зенит, переходящий в тёплый закатный горизонт
	sky_top_color            = Color(0.071, 0.082, 0.165, 1.0)
	sky_horizon_color        = Color(0.671, 0.318, 0.231, 1.0)  # Тёплый закатный отсвет
	sky_ground_horizon_color = Color(0.522, 0.227, 0.180, 1.0)  # Глубокий пурпурно-красный
	sky_ground_bottom_color  = Color(0.118, 0.110, 0.188, 1.0)  # Тёмно-синий снег в тени
	sky_sun_angle_max        = 8.0   # Солнце почти за горизонтом
	sky_sun_curve            = 0.08

	# Ambient — холодный синий с тёплым отсветом заката
	ambient_color            = Color(0.412, 0.357, 0.557, 1.0)
	ambient_sky_contribution = 0.65
	ambient_energy           = 0.35

	# Туман — лёгкий морозный, чуть голубоватый
	fog_enabled              = true
	fog_color                = Color(0.380, 0.318, 0.451, 1.0)
	fog_light_color          = Color(0.820, 0.502, 0.322, 1.0)  # Закатный отсвет в тумане
	fog_sun_scatter          = 0.55  # Активное рассеивание закатного света
	fog_density              = 0.018
	fog_aerial_perspective   = 0.75

	# Объёмный туман — лёгкое свечение в воздухе
	volumetric_fog_enabled   = true
	volumetric_fog_density   = 0.012

	# Тональная коррекция — кинематографичная, тёплая
	tonemap_mode             = 2     # Filmic
	tonemap_exposure         = 0.78
	tonemap_white            = 8.0

	# Bloom — магическое свечение (фонари, снег, закат)
	glow_enabled             = true
	glow_intensity           = 1.4
	glow_bloom               = 0.22

	# SSAO — мягкие тени в снегу и деревьях
	ssao_enabled             = true
	ssao_intensity           = 1.4

	# Цветокоррекция — насыщенные закатные цвета, чуть приглушённые
	adjustment_brightness    = 0.88
	adjustment_contrast      = 1.12
	adjustment_saturation    = 0.82

	# Солнце — низкое, оранжево-красное, почти за горизонтом
	sun_color                = Color(1.0, 0.447, 0.196, 1.0)
	sun_energy               = 1.8   # Яркое, но низкое — длинные тени
	sun_angular_distance     = 1.8   # Чуть увеличен диск у горизонта
