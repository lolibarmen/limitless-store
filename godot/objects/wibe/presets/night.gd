# presets/night.gd
extends AtmospherePreset

func _init() -> void:
	sky_top_color            = Color(0.012, 0.020, 0.055, 1.0)
	sky_horizon_color        = Color(0.050, 0.070, 0.130, 1.0)
	sky_ground_horizon_color = Color(0.030, 0.040, 0.070, 1.0)
	sky_ground_bottom_color  = Color(0.010, 0.015, 0.030, 1.0)
	sky_sun_angle_max        = 10.0
	sky_sun_curve            = 0.05

	ambient_color            = Color(0.150, 0.180, 0.280, 1.0)
	ambient_sky_contribution = 0.5
	ambient_energy           = 0.15

	fog_enabled              = true
	fog_color                = Color(0.030, 0.045, 0.080, 1.0)
	fog_light_color          = Color(0.100, 0.130, 0.220, 1.0)
	fog_sun_scatter          = 0.05
	fog_density              = 0.015
	fog_aerial_perspective   = 0.8

	volumetric_fog_enabled   = true
	volumetric_fog_density   = 0.02

	tonemap_mode             = 2
	tonemap_exposure         = 0.5
	tonemap_white            = 4.0

	glow_enabled             = true
	glow_intensity           = 1.5
	glow_bloom               = 0.2

	ssao_enabled             = true
	ssao_intensity           = 2.5

	adjustment_brightness    = 0.75
	adjustment_contrast      = 1.15
	adjustment_saturation    = 0.7

	sun_color                = Color(0.700, 0.780, 0.900, 1.0)
	sun_energy               = 0.15
	sun_angular_distance     = 0.3
