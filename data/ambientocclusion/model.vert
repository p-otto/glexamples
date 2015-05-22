#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;
out float v_depth;

uniform mat4 u_mvp;
uniform mat4 u_view;

void main()
{
    v_normal = (u_view * vec4(a_normal, 0.0)).xyz;
    v_normal = normalize(v_normal);
    vec4 view_pos = u_view * vec4(a_vertex, 1.0);
    v_depth = view_pos.z;
    gl_Position = u_mvp * vec4(a_vertex, 1.0);
}
