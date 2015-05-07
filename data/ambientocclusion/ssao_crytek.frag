#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_depth;
uniform sampler2D u_normal;

layout(location = 0) out float occlusion;

#define KERNEL_SIZE 5
#define EPSILON 0.01

void main()
{
    float cur_depth = texture(u_depth, v_uv).r;
    occlusion = 0.0;
    for (int x = -KERNEL_SIZE; x <= KERNEL_SIZE; ++x)
    {
        for (int y = -KERNEL_SIZE; y <= KERNEL_SIZE; ++y)
        {
            float cmp_depth = texture(u_depth, v_uv + vec2(x * EPSILON, y * EPSILON)).r;
            if (cmp_depth < cur_depth) {
                occlusion += 1.0;
            }
        }
    }
    occlusion /= (KERNEL_SIZE * 2 + 1) * (KERNEL_SIZE * 2 + 1);
}
