#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec4 v_position;
in vec3 v_normal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 normal_depth;

void main()
{
    vec3 norm_normal = normalize(v_normal);
    normal_depth = vec4(norm_normal * 0.5 + 0.5, v_position.z / v_position.w);
    fragColor = vec4(1.0);
}
