// particleShader (vs) - renders particle systems
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 mixColor;

layout (location = 1) in vec4 spriteColor;
layout (location = 2) in vec3 pos;
layout (location = 3) in float scale;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 spriteDims;

void main() {
    TexCoords = vertex.zw;
	mixColor = spriteColor;
    gl_Position = projection * view * vec4(pos,1.0);
	// offset position in clip space by sprite dimensions
	// subtract spriteDims/2 to get centered rendering
	// apply the projection to the offset as well
	// TODO: particles appear to be flipped vertically
	gl_Position.xy += (projection * vec4(spriteDims*vertex.xy * scale - (scale*spriteDims/2),0,0)).xy;
}