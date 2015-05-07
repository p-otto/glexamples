#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec3 v_normal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 normal;

const vec3 light_dir = vec3(0.0, 1.0, 0.0);

void main()
{
    vec3 norm_normal = normalize(v_normal);
    normal = vec3(1.0, 0.0, 0.0);//norm_normal * 0.5 + 0.5;
    float diffuse_factor = max(0, dot(norm_normal, light_dir));
    fragColor = diffuse_factor * vec4(1.0);
}
