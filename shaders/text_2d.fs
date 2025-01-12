#version 460 core

in vec2 f_tex_coords;
out vec4 out_color;

uniform sampler2D u_text_sampler;
uniform vec3 u_text_color;

void main()
{
    vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(u_text_sampler, f_tex_coords).r);
    out_color = vec4(u_text_color, 1.0f) * sampled;
}
