# presets/dense_fog.gd
extends AtmospherePreset

func _init() -> void:
	sky_top_color            = Color(0.600, 0.620, 0.630, 1.0)
	sky_horizon_color        = Color(0.750, 0.760, 0.770, 1.0)
	sky_ground_horizon_color = Color(0.750, 0.760, 0.770, 1.0)
	sky_ground_bottom_color  = Color(0.700, 0.710, 0.720, 1.0)
	sky_sun_angle_max        = 20.0
	sky_sun_curve            = 0.3

	ambient_color            = Color(0.720, 0.730, 0.740, 1.0)
	ambient_sky_contribution = 1.0
	ambient_energy           = 0.5

	fog_enabled              = true
	fog_color                = Color(0.750, 0.760, 0.770, 1.0)
	fog_light_color          = Color(0.800, 0.800, 0.800, 1.0)
	fog_sun_scatter          = 0.05
	fog_density              = 0.12
	fog_aerial_perspective   = 1.0

	volumetric_fog_enabled   = true
	volumetric_fog_density   = 0.08

	tonemap_mode             = 1
	tonemap_exposure         = 0.9
	tonemap_white            = 5.0

	glow_enabled             = true
	glow_intensity           = 0.6
	glow_bloom               = 0.3

	ssao_enabled             = false
	ssao_intensity           = 1.0

	adjustment_brightness    = 1.0
	adjustment_contrast      = 0.95
	adjustment_saturation    = 0.45

	sun_color                = Color(0.900, 0.880, 0.840, 1.0)
	sun_energy               = 0.3
	sun_angular_distance     = 3.0
