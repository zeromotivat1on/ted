#version 460 core

layout (location = 0) in vec3 vpos;

void main()
{
    gl_Position = vec4(vpos.x, vpos.y, vpos.z, 0);
}
