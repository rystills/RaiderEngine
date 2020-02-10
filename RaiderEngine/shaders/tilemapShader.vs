#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;
uniform vec2 pos;

void main() {
    gl_Position = projection * vec4(pos + vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}  