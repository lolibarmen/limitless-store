extends Node3D

# =============================================================================
#  AtmosphereController.gd
#  Прикрепи к корню сцены. Загружает пресеты из папки presets_path.
#
#  Структура файлов:
#  res://objects/wibe/
#    ├── AtmosphereController.gd   (этот файл)
#    ├── AtmospherePreset.gd       (класс ресурса)
#    └── presets/
#          ├── winter_fog.tres
#          ├── summer_day.tres
#          ├── night.tres
#          └── dense_fog.tres
# =============================================================================

@export var presets_path: String = "res://objects/wibe/presets/"
@export var active_preset: String = "winter_fog"
@export var transition_duration: float = 2.0

@onready var world_env: WorldEnvironment = $WorldEnvironment
@onready var sun: DirectionalLight3D = $DirectionalLight3D

var presets: Dictionary = {}          # { "winter_fog": AtmospherePreset, ... }
var current_preset_name: String = ""
var _tween: Tween


# =============================================================================
#  ЗАГРУЗКА ПРЕСЕТОВ
# =============================================================================
func _load_presets() -> void:
	var dir := DirAccess.open(presets_path)
	if dir == null:
		push_error("AtmosphereController: папка пресетов не найдена: %s" % presets_path)
		return

	dir.list_dir_begin()
	var file_name := dir.get_next()

	while file_name != "":
		if not dir.current_is_dir() and file_name.ends_with(".gd"):
			var full_path := presets_path + file_name
			var script := load(full_path)
			var instance: AtmospherePreset = script.new()
			if instance is AtmospherePreset:
				var key := file_name.get_basename()   # "winter_fog.gd" → "winter_fog"
				presets[key] = instance
				print("AtmosphereController: загружен пресет '%s'" % key)
			else:
				push_warning("AtmosphereController: файл '%s' не является AtmospherePreset, пропускаю." % file_name)
		file_name = dir.get_next()

	dir.list_dir_end()
	print("AtmosphereController: загружено пресетов — %d: %s" % [presets.size(), presets.keys()])


# =============================================================================
#  ПРИМЕНЕНИЕ ПРЕСЕТА
# =============================================================================
func apply_preset(preset_name: String, instant: bool = false) -> void:
	if not presets.has(preset_name):
		push_error("AtmosphereController: пресет '%s' не найден. Доступны: %s" % [preset_name, presets.keys()])
		return

	var p: AtmospherePreset = presets[preset_name]
	current_preset_name = preset_name

	if instant or transition_duration <= 0.0:
		_apply_instant(p)
	else:
		_apply_tweened(p)

	print("AtmosphereController: активирован пресет '%s'" % preset_name)


func _apply_instant(p: AtmospherePreset) -> void:
	var env     := world_env.environment
	var sky_mat := env.sky.sky_material as ProceduralSkyMaterial

	sky_mat.sky_top_color            = p.sky_top_color
	sky_mat.sky_horizon_color        = p.sky_horizon_color
	sky_mat.ground_horizon_color     = p.sky_ground_horizon_color
	sky_mat.ground_bottom_color      = p.sky_ground_bottom_color
	sky_mat.sun_angle_max            = p.sky_sun_angle_max
	sky_mat.sun_curve                = p.sky_sun_curve

	env.ambient_light_color              = p.ambient_color
	env.ambient_light_sky_contribution   = p.ambient_sky_contribution
	env.ambient_light_energy             = p.ambient_energy

	env.fog_enabled                  = p.fog_enabled
	env.fog_light_color              = p.fog_light_color
	env.fog_sun_scatter              = p.fog_sun_scatter
	env.fog_density                  = p.fog_density
	env.fog_aerial_perspective       = p.fog_aerial_perspective

	env.volumetric_fog_enabled       = p.volumetric_fog_enabled
	env.volumetric_fog_density       = p.volumetric_fog_density

	env.tonemap_mode                 = p.tonemap_mode
	env.tonemap_exposure             = p.tonemap_exposure
	env.tonemap_white                = p.tonemap_white

	env.glow_enabled                 = p.glow_enabled
	env.glow_intensity               = p.glow_intensity
	env.glow_bloom                   = p.glow_bloom

	env.ssao_enabled                 = p.ssao_enabled
	env.ssao_intensity               = p.ssao_intensity

	env.adjustment_enabled           = true
	env.adjustment_brightness        = p.adjustment_brightness
	env.adjustment_contrast          = p.adjustment_contrast
	env.adjustment_saturation        = p.adjustment_saturation

	sun.light_color                  = p.sun_color
	sun.light_energy                 = p.sun_energy
	sun.light_angular_distance       = p.sun_angular_distance


func _apply_tweened(p: AtmospherePreset) -> void:
	if _tween:
		_tween.kill()
	_tween = create_tween().set_parallel(true)

	var env     := world_env.environment
	var sky_mat := env.sky.sky_material as ProceduralSkyMaterial
	var dur     := transition_duration

	_tween.tween_property(sky_mat, "sky_top_color",        p.sky_top_color,            dur)
	_tween.tween_property(sky_mat, "sky_horizon_color",    p.sky_horizon_color,        dur)
	_tween.tween_property(sky_mat, "ground_horizon_color", p.sky_ground_horizon_color, dur)
	_tween.tween_property(sky_mat, "ground_bottom_color",  p.sky_ground_bottom_color,  dur)

	_tween.tween_property(env, "ambient_light_color",            p.ambient_color,            dur)
	_tween.tween_property(env, "ambient_light_sky_contribution", p.ambient_sky_contribution, dur)
	_tween.tween_property(env, "ambient_light_energy",           p.ambient_energy,           dur)

	_tween.tween_property(env, "fog_light_color",        p.fog_light_color,        dur)
	_tween.tween_property(env, "fog_density",            p.fog_density,            dur)
	_tween.tween_property(env, "fog_aerial_perspective", p.fog_aerial_perspective, dur)

	_tween.tween_property(env, "tonemap_exposure", p.tonemap_exposure, dur)
	_tween.tween_property(env, "tonemap_white",    p.tonemap_white,    dur)

	_tween.tween_property(env, "glow_intensity",        p.glow_intensity,        dur)
	_tween.tween_property(env, "adjustment_brightness", p.adjustment_brightness, dur)
	_tween.tween_property(env, "adjustment_contrast",   p.adjustment_contrast,   dur)
	_tween.tween_property(env, "adjustment_saturation", p.adjustment_saturation, dur)

	_tween.tween_property(sun, "light_color",  p.sun_color,  dur)
	_tween.tween_property(sun, "light_energy", p.sun_energy, dur)

	# Булевые и enum — применяем мгновенно
	env.fog_enabled            = p.fog_enabled
	env.fog_sun_scatter        = p.fog_sun_scatter
	env.volumetric_fog_enabled = p.volumetric_fog_enabled
	env.volumetric_fog_density = p.volumetric_fog_density
	env.tonemap_mode           = p.tonemap_mode
	env.glow_enabled           = p.glow_enabled
	env.glow_bloom             = p.glow_bloom
	env.ssao_enabled           = p.ssao_enabled
	env.ssao_intensity         = p.ssao_intensity
	env.adjustment_enabled     = true
	sun.light_angular_distance = p.sun_angular_distance


# =============================================================================
#  ПУБЛИЧНОЕ API
# =============================================================================

func cycle_preset() -> void:
	var keys := presets.keys()
	var idx  := keys.find(current_preset_name)
	var next: String = keys[(idx + 1) % keys.size()]
	apply_preset(next)

func get_preset_names() -> Array:
	return presets.keys()


# =============================================================================
#  ИНИЦИАЛИЗАЦИЯ
# =============================================================================
func _ready() -> void:
	_load_presets()
	apply_preset(active_preset, true)


# =============================================================================
#  DEBUG: F5 — цикл пресетов
# =============================================================================
func _input(event: InputEvent) -> void:
	if OS.is_debug_build() and event is InputEventKey and event.pressed:
		if event.keycode == KEY_F5:
			cycle_preset()
			#print("Пресет: ", current_preset_name)
