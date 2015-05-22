#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec2 a_vertex;
out vec2 v_uv;
out vec3 v_viewRay;

uniform mat4 u_invProj;

void main()
{
    v_uv = a_vertex * 0.5 + 0.5;
    vec4 viewRay4 = u_invProj * vec4(a_vertex, 1.0, 1.0);
    v_viewRay = viewRay4.xyz / viewRay4.w;
    gl_Position = vec4(a_vertex, 0.0, 1.0);
}
