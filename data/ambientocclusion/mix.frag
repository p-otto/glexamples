#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_color;
uniform sampler2D u_blur;
uniform sampler2D u_normal_depth;
uniform sampler2D u_depth;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 color = texture(u_color, v_uv).rgb;
    float occlusion = texture(u_blur, v_uv).r;

    gl_FragDepth = texture(u_depth, v_uv).r;
    fragColor = vec4(color * occlusion, 1.0);
}
