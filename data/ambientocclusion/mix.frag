#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec2 v_uv;

uniform sampler2D u_ambient;
uniform sampler2D u_diffuse;
uniform sampler2D u_blur;
uniform sampler2D u_depth;

uniform float u_ambient_factor;
uniform bool u_packed_occlusion;

layout(location = 0) out vec4 fragColor;

#include "/utility"

void main()
{
    vec3 ambient = texture(u_ambient, v_uv).rgb;
    vec3 diffuse = texture(u_diffuse, v_uv).rgb;
    vec3 occlusion = texture(u_blur, v_uv).rgb;

    // for SSDO the occlusion and color bleeding values are packed into one texture
    // so they are unpacked here
    if (u_packed_occlusion) {
        occlusion *= 2.0;
    }

    gl_FragDepth = texture(u_depth, v_uv).r;
    fragColor = vec4(evaluatePhong(u_ambient_factor * ambient, (1.0 - u_ambient_factor) * diffuse, occlusion), 1.0);
}
