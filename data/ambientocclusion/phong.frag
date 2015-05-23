#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec3 v_normal;
in float v_depth;
in vec3 v_worldPos;
in vec3 v_worldNormal;

uniform float u_farPlane;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 normal_depth;

const vec3 light = vec3(8.0,12.0,-8.0);
const vec3 ambient = vec3(0.3, 0.3, 0.3);

void main()
{
    vec3 norm_normal = normalize(v_normal);
    normal_depth = vec4(norm_normal * 0.5 + 0.5,
                        -v_depth / u_farPlane);
    float factor = max(dot(normalize(v_worldNormal), normalize(light - v_worldPos)), 0.0);
    fragColor = vec4(vec3(1.0) * factor  + ambient, 1.0);
}
