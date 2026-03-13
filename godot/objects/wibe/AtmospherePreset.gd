# AtmospherePreset.gd
# Положи этот файл в res://objects/wibe/AtmospherePreset.gd
# Каждый пресет — отдельный .tres файл на основе этого класса.
# Создать пресет: ПКМ в FileSystem → New Resource → AtmospherePreset

class_name AtmospherePreset
extends Resource

@export_group("Sky")
@export var sky_top_color: Color            = Color(0.168, 0.212, 0.271, 1.0)
@export var sky_horizon_color: Color        = Color(0.561, 0.627, 0.690, 1.0)
@export var sky_ground_horizon_color: Color = Color(0.784, 0.831, 0.863, 1.0)
@export var sky_ground_bottom_color: Color  = Color(0.784, 0.831, 0.863, 1.0)
@export var sky_sun_angle_max: float        = 30.0
@export var sky_sun_curve: float            = 0.15

@export_group("Ambient")
@export var ambient_color: Color            = Color(0.690, 0.769, 0.831, 1.0)
@export var ambient_sky_contribution: float = 0.8
@export var ambient_energy: float           = 0.4

@export_group("Fog")
@export var fog_enabled: bool               = true
@export var fog_color: Color                = Color(0.659, 0.737, 0.784, 1.0)
@export var fog_light_color: Color          = Color(0.659, 0.737, 0.784, 1.0)
@export var fog_sun_scatter: float          = 0.1
@export var fog_density: float              = 0.025
@export var fog_aerial_perspective: float   = 0.6

@export_group("Volumetric Fog")
@export var volumetric_fog_enabled: bool    = false
@export var volumetric_fog_density: float   = 0.0

@export_group("Tonemap")
@export_enum("Linear", "Reinhardt", "Filmic", "ACES") var tonemap_mode: int = 2
@export var tonemap_exposure: float         = 0.85
@export var tonemap_white: float            = 6.0

@export_group("Glow")
@export var glow_enabled: bool              = true
@export var glow_intensity: float           = 0.8
@export var glow_bloom: float               = 0.1

@export_group("SSAO")
@export var ssao_enabled: bool              = true
@export var ssao_intensity: float           = 1.8

@export_group("Color Adjustment")
@export var adjustment_brightness: float    = 0.95
@export var adjustment_contrast: float      = 1.05
@export var adjustment_saturation: float    = 0.55

@export_group("Sun")
@export var sun_color: Color                = Color(0.784, 0.847, 0.910, 1.0)
@export var sun_energy: float               = 0.6
@export var sun_angular_distance: float     = 1.0
