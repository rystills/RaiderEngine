#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 mixColor;

layout (location = 1) in mat4 model;
layout (location = 5) in vec3 spriteColor;
uniform mat4 projection;

void main() {
    TexCoords = vertex.zw;
	mixColor = vec4(spriteColor, 1.0);
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}