// point_shadows_depth (vs) - renders point light shadows to a depth cubemap
#version 330 core
layout (location = 0) in vec3 aPos;

layout (location = 5) in mat4 instanceMatrix;

void main()
{
    gl_Position = instanceMatrix * vec4(aPos, 1.0);
}