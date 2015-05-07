#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec4 v_position;
out vec3 v_normal;

uniform mat4 transform;

void main()
{
    v_normal = a_normal;
    vec4 out_pos = transform * vec4(a_vertex, 1.0);
    v_position = out_pos;
    gl_Position = out_pos;
}
