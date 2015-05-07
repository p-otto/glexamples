#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_color;
uniform sampler2D u_blur;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = mix(
        vec4(texture(u_color, v_uv).rgb, 1.0),
        vec4(texture(u_blur, v_uv).rrr, 1.0),
        0.5
    );
}
