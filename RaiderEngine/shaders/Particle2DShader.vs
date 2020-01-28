#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 mixColor;

layout (location = 1) in vec3 spriteColor;
layout (location = 2) in vec2 pos;
layout (location = 3) in float scale;
uniform mat4 projection;
uniform vec2 spriteDims;

void main() {
    TexCoords = vertex.zw;
	mixColor = vec4(spriteColor, 1.0);
    gl_Position = projection * vec4(pos.xy + spriteDims*vertex.xy * scale,0.0,1.0);
}