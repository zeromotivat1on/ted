#version 460 core

in vec2 f_tex_coords;
in flat int f_instance_id;
out vec4 out_color;

uniform sampler2DArray u_text_sampler_array;
uniform unsigned int u_charmap[128];
uniform vec3 u_text_color;

void main()
{
    vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(u_text_sampler_array, vec3(f_tex_coords.xy, u_charmap[f_instance_id])).r);
    out_color = vec4(u_text_color, 1.0f) * sampled;
}
