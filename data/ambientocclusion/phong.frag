#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec3 v_normal;
in float v_depth;
in vec3 v_worldPos;
in vec3 v_worldNormal;

uniform float u_nearPlane;
uniform float u_farPlane;
uniform vec3 u_color;


layout(location = 0) out vec3 ambientColor;
layout(location = 1) out vec3 diffuseColor;
layout(location = 2) out vec4 normal_depth;

#include "/lights"

void main()
{
    vec3 norm_normal = normalize(v_normal);
    normal_depth = vec4(norm_normal * 0.5 + 0.5,
                        (-v_depth - u_nearPlane) / (u_farPlane - u_nearPlane));

    // only use ambient and diffuse terms
    vec3 diffuse = vec3(0.0);
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
    	vec3 pos_to_light = normalize(light_positions[i] - v_worldPos);
    	float light_normal_cos = max(0.0, dot(normalize(v_worldNormal), pos_to_light));
    	diffuse += light_normal_cos * light_colors[i];
    }
    diffuse = clamp(diffuse, 0.0, 1.0);

    ambientColor = vec3(u_color);
    diffuseColor = vec3(diffuse * u_color);
}
