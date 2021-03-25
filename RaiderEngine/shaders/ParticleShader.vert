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
	// subtract spriteDims/2 to get centered rendering
//    gl_Position = projection * view * vec4(pos.xy + spriteDims*vertex.xy * scale - (scale*spriteDims/2),pos.z,1.0);
//    gl_Position = projection * vec4(pos.xy + spriteDims*vertex.xy * scale - (scale*spriteDims/2),.1,1.0);
//	gl_Position = projection * view * vec4(pos, 1.0);

	gl_Position = projection * vec4(pos.xy + spriteDims*vertex.xy * scale - (scale*spriteDims/2),.1,1.0);
}