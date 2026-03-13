# presets/winter_fog.gd
extends AtmospherePreset

func _init() -> void:
	sky_top_color            = Color(0.168, 0.212, 0.271, 1.0)
	sky_horizon_color        = Color(0.561, 0.627, 0.690, 1.0)
	sky_ground_horizon_color = Color(0.784, 0.831, 0.863, 1.0)
	sky_ground_bottom_color  = Color(0.784, 0.831, 0.863, 1.0)
	sky_sun_angle_max        = 30.0
	sky_sun_curve            = 0.15

	ambient_color            = Color(0.690, 0.769, 0.831, 1.0)
	ambient_sky_contribution = 0.8
	ambient_energy           = 0.4

	fog_enabled              = true
	fog_color                = Color(0.659, 0.737, 0.784, 1.0)
	fog_light_color          = Color(0.659, 0.737, 0.784, 1.0)
	fog_sun_scatter          = 0.1
	fog_density              = 0.025
	fog_aerial_perspective   = 0.6

	volumetric_fog_enabled   = false
	volumetric_fog_density   = 0.0

	tonemap_mode             = 2
	tonemap_exposure         = 0.85
	tonemap_white            = 6.0

	glow_enabled             = true
	glow_intensity           = 0.8
	glow_bloom               = 0.1

	ssao_enabled             = true
	ssao_intensity           = 1.8

	adjustment_brightness    = 0.95
	adjustment_contrast      = 1.05
	adjustment_saturation    = 0.55

	sun_color                = Color(0.784, 0.847, 0.910, 1.0)
	sun_energy               = 0.6
	sun_angular_distance     = 1.0
