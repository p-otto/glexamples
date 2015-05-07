#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_occlusion;

layout(location = 0) out float occlusion;

#define KERNEL_SIZE 3
#define EPSILON 0.005

void main()
{
    occlusion = 0.0;
    for (int x = -KERNEL_SIZE; x <= KERNEL_SIZE; ++x)
    {
        for (int y = -KERNEL_SIZE; y <= KERNEL_SIZE; ++y)
        {
            occlusion += texture(u_occlusion, v_uv + vec2(x * EPSILON, y * EPSILON)).r;
        }
    }
    occlusion /= (KERNEL_SIZE * 2 + 1) * (KERNEL_SIZE * 2 + 1);
}
