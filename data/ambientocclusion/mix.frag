#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec3 v_uv;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
