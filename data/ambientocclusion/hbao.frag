#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

#define MAX_KERNEL_SIZE 128
uniform float kernel[MAX_KERNEL_SIZE];

layout(location = 0) out float occlusion;

void main()
{
    occlusion = 0.0;
}
