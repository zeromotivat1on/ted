#version 460 core

layout (location = 0) in vec3 vpos;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vpos.xyz, 1.0f);
}
