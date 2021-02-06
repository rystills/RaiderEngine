// particle2DShader (fs) - renders 2D particle systems
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
in vec4 mixColor;

void main() {
    if( texture(image, TexCoords).a < .001)
        discard;
    color = mixColor * texture(image, TexCoords);
}  