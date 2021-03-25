// particleShader (fs) - renders particle systems
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
in vec4 mixColor;

void main() {
    vec4 col =  texture(image, TexCoords);
    if (col.a < .001)
        discard;
    color = mixColor * col;
}  