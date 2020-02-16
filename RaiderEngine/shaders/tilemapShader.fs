#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;

void main() {
    if( texture(text, TexCoords).a < .001)
        discard;
    color = texture(text, TexCoords);
}  