#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec4 color_from_vshader;
uniform mat4 projection;

void main() {
	gl_PointSize = 3;
	gl_Position = projection * vec4(position, 1.0f);
	color_from_vshader = color;
}