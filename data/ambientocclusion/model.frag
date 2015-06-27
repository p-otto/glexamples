#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec3 v_normal;
in float v_depth;

uniform float u_farPlane;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 normal_depth;

void main()
{
    vec3 norm_normal = normalize(v_normal);
    normal_depth = vec4(norm_normal * 0.5 + 0.5,
                        -v_depth / u_farPlane);
    fragColor = vec4(1.0);
}
