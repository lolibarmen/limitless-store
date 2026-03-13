# presets/summer_day.gd
extends AtmospherePreset

func _init() -> void:
	sky_top_color            = Color(0.251, 0.478, 0.800, 1.0)
	sky_horizon_color        = Color(0.600, 0.780, 0.950, 1.0)
	sky_ground_horizon_color = Color(0.420, 0.600, 0.300, 1.0)
	sky_ground_bottom_color  = Color(0.300, 0.450, 0.200, 1.0)
	sky_sun_angle_max        = 90.0
	sky_sun_curve            = 0.08

	ambient_color            = Color(0.800, 0.820, 0.780, 1.0)
	ambient_sky_contribution = 0.9
	ambient_energy           = 0.6

	fog_enabled              = false
	fog_color                = Color(0.700, 0.820, 0.950, 1.0)
	fog_light_color          = Color(1.0, 0.980, 0.900, 1.0)
	fog_sun_scatter          = 0.3
	fog_density              = 0.002
	fog_aerial_perspective   = 0.2

	volumetric_fog_enabled   = false
	volumetric_fog_density   = 0.0

	tonemap_mode             = 2
	tonemap_exposure         = 1.0
	tonemap_white            = 8.0

	glow_enabled             = true
	glow_intensity           = 1.0
	glow_bloom               = 0.05

	ssao_enabled             = true
	ssao_intensity           = 1.2

	adjustment_brightness    = 1.05
	adjustment_contrast      = 1.0
	adjustment_saturation    = 1.1

	sun_color                = Color(1.0, 0.960, 0.820, 1.0)
	sun_energy               = 1.8
	sun_angular_distance     = 0.5
