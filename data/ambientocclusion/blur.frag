#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_normal_depth;
uniform sampler2D u_occlusion;

layout(location = 0) out float occlusion;

#define KERNEL_SIZE 2
#define EPSILON 0.001

void main()
{
    float depth_diff_x = texture(u_normal_depth, v_uv - vec2(-EPSILON, 0.0)).a - texture(u_normal_depth, v_uv - vec2(EPSILON, 0.0)).a;
    float depth_diff_y = texture(u_normal_depth, v_uv - vec2(0.0, -EPSILON)).a - texture(u_normal_depth, v_uv - vec2(0.0, EPSILON)).a;

    float diff = sqrt(abs(depth_diff_x) + abs(depth_diff_y));
    diff = clamp(diff, 0.0, 0.05);
    diff /= 0.05;

    float epsilon = mix(0.0001, 0.001, 1 - diff);
    occlusion = 0.0;
    for (int x = -KERNEL_SIZE; x <= KERNEL_SIZE; ++x)
    {
        for (int y = -KERNEL_SIZE; y <= KERNEL_SIZE; ++y)
        {
            occlusion += texture(u_occlusion, v_uv + vec2(x * epsilon, y * epsilon)).r;
        }
    }
    occlusion /= (KERNEL_SIZE * 2 + 1) * (KERNEL_SIZE * 2 + 1);
}
