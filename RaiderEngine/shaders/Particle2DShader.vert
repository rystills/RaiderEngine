// particle2DShader (vs) - renders 2D particle systems
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
out vec4 mixColor;

layout (location = 1) in vec4 spriteColor;
layout (location = 2) in vec2 pos;
layout (location = 3) in float scale;
uniform mat4 projection;
uniform vec2 spriteDims;
uniform float depth;

void main() {
    TexCoords = vertex.zw;
	mixColor = spriteColor;
	// offset position by sprite dimensions
	// subtract spriteDims/2 to get centered rendering
    gl_Position = projection * vec4(pos.xy + spriteDims*vertex.xy * scale - (scale*spriteDims/2),depth,1.0);
}