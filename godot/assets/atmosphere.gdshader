shader_type spatial;
render_mode unshaded;

uniform vec3 atmosphere_color : source_color = vec3(0.3, 0.6, 1.0);
uniform vec3 sun_color        : source_color = vec3(1.0, 0.5, 0.2);
uniform vec3 sun_direction = vec3(0.0, 1.0, 0.0); // нормализованный вектор к солнцу

uniform float density        = 1.5;
uniform float horizon_falloff = 4.0;
uniform float sun_spread      = 8.0;  // чем больше — тем уже пятно
uniform float sun_strength    = 1.0;  // интенсивность солнечного цвета

void fragment() {
    vec3 normal = NORMAL;
    vec3 view   = VIEW;

    // Рим-эффект (граница атмосферы)
    float rim = 1.0 - abs(dot(normal, view));
    float alpha = pow(rim, horizon_falloff) * density;

    // Направление взгляда в мировом пространстве
    // VIEW — от камеры к фрагменту, нам нужно обратное
    vec3 view_world = normalize((INV_VIEW_MATRIX * vec4(-VIEW, 0.0)).xyz);

    // Угол между направлением взгляда и направлением на солнце
    float sun_dot = max(0.0, dot(view_world, normalize(sun_direction)));

    // Чем ближе смотрим к солнцу — тем сильнее солнечный цвет
    float sun_factor = pow(sun_dot, sun_spread) * sun_strength;

    vec3 final_color = mix(atmosphere_color, sun_color, sun_factor);

    ALBEDO = final_color;
    ALPHA  = clamp(alpha, 0.0, 1.0);
}