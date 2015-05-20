#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_normal_depth;
uniform sampler2D u_occlusion;
uniform int u_kernelSize;

layout(location = 0) out float occlusion;

#define EPSILON 0.01

void main()
{
    float depth_diff_x = texture(u_normal_depth, v_uv - vec2(-EPSILON, 0.0)).a - texture(u_normal_depth, v_uv - vec2(EPSILON, 0.0)).a;
    float depth_diff_y = texture(u_normal_depth, v_uv - vec2(0.0, -EPSILON)).a - texture(u_normal_depth, v_uv - vec2(0.0, EPSILON)).a;

    float diff = abs(depth_diff_x) + abs(depth_diff_y);
    diff = clamp(diff / 2.0, 0.0, 0.01);
    diff /= 0.01;

    float epsilon = mix(0.001, 0.0003, diff);
    occlusion = 0.0;
    for (int y = -u_kernelSize; y <= u_kernelSize; ++y)
    {
        occlusion += texture(u_occlusion, v_uv + vec2(0, y * epsilon)).r;
    }
    occlusion /= u_kernelSize * 2 + 1;
}
