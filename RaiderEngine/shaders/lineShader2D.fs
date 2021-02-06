// lineShader2D (fs) - handles 2D line rendering
#version 330 core
in vec4 color_from_vshader;
out vec4 out_color;

void main() {
	out_color = color_from_vshader;
}