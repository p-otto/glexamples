#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

layout(location = 0) out vec3 occlusion;

void main()
{
    occlusion = vec3(1.0);
}
