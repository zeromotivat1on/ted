#version 460 core

layout (location = 0) in vec2 vpos;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vpos.xy, 0.0f, 1.0f);
}
