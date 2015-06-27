#define LIGHT_COUNT 2
const vec3 light_positions[LIGHT_COUNT] = vec3[](vec3(-15.0, 30.0, -25.0), vec3(15.0, 30.0, -25.0));
const vec3 light_colors[LIGHT_COUNT] = vec3[](vec3(1.0, 0.97, 0.4), vec3(0.0, 0.0, 1.0));
const float attenuation_factor = 0.0005;