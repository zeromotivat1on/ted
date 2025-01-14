#version 460 core

layout (location = 0) in vec2 v_vertex; // vec2 pos

out vec2 f_tex_coords;
out flat int f_instance_id;

uniform mat4 u_transforms[128];
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_transforms[gl_InstanceID] * vec4(v_vertex.xy, 0.0f, 1.0f);
    f_tex_coords = v_vertex.xy;
    f_tex_coords.y = 1.0f - v_vertex.y; // vertical flip
    f_instance_id = gl_InstanceID;
}
