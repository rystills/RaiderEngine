// point_shadows_depth (vs) - renders point light shadows to a depth cubemap
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in mat4 instanceMatrix;

out vec2 texCoordsInitial;

void main()
{
    gl_Position = instanceMatrix * vec4(aPos, 1.0);
    texCoordsInitial = aTexCoords;
}