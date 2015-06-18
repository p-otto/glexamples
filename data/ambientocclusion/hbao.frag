#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

layout(location = 0) out float occlusion;

void main()
{
    occlusion = 0.0;
}
