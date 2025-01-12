#version 460 core

layout (location = 0) in vec4 v_vertex; // vec2 pos, vec2 tex_coords

out vec2 f_tex_coords;

uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * vec4(v_vertex.xy, 0.0f, 1.0f);
    f_tex_coords = v_vertex.zw;
}
