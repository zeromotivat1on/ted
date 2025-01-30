#version 460 core

layout (location = 0) in vec2 v_vertex; // vec2 pos

uniform mat4 u_transform;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_transform * vec4(v_vertex.xy, 0.0f, 1.0f);
}
