#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_depth;
uniform sampler2D u_normal;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture(u_normal, v_uv);
}
