#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor, ourPos;
uniform float xOff;

void main()
{
    gl_Position = vec4(aPos.x+xOff,-aPos.y,aPos.z, 1.0);
    ourColor = aColor;
	ourPos = aPos;
}