shader_type spatial;
render_mode unshaded, cull_disabled, depth_prepass_alpha, depth_draw_always, fog_disabled;

uniform sampler2D albedo_tex : source_color, filter_nearest;

// Ветер
uniform vec2 wind_direction = vec2(1.0, 0.35);
uniform float wind_speed = 2.0;    // скорость бега волны
uniform float wind_power = 0.22;   // сила изгиба
uniform float wave_scale = 3.0;    // длина волны (больше = шире "аниме-волна")
uniform float gust_strength = 0.4; // порывы

// Цвет (например, тон времени суток — один uniform, почти бесплатно)
uniform vec4 tint : source_color = vec4(1.0);

void vertex() {
    vec3 wp = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;

    // Маска изгиба: 0 у корня, 1 у кончика.
    // Если гнётся корень вместо верха — замените на (UV.y)
    float mask = 1.0 - UV.y;
    mask *= mask;

    vec2 dir = normalize(wind_direction);
    // Фаза от мировой позиции => волна бежит ПО ПОЛЮ, соседи качаются синхронно
    float phase = dot(wp.xz, dir) / wave_scale - TIME * wind_speed;
    phase += INSTANCE_CUSTOM.r * 6.2831; // личный сдвиг инстанса

    float wave = sin(phase) * 0.7 + sin(phase * 2.3 + 1.7) * 0.3;
    float gust = sin(phase * 0.37 + 2.0) * gust_strength;
    float bend = (wave + gust) * wind_power;

    VERTEX += vec3(dir.x, 0.0, dir.y) * bend * mask;
    VERTEX.y -= abs(bend) * mask * 0.3; // чтобы трава не "всплывала" при наклоне
}

void fragment() {
    vec4 c = texture(albedo_tex, UV);
    ALBEDO = c.rgb * tint.rgb;
    ALPHA = c.a;
    ALPHA_SCISSOR_THRESHOLD = 0.5; // жёсткий край: дёшево + подходит пиксель-арту
}