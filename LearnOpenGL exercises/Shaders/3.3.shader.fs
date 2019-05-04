#version 330 core
out vec4 FragColor;

in vec3 ourColor, ourPos;

void main()
{
    FragColor = vec4(ourPos, 1.0f);
}