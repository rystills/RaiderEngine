// g_buffer (vs) - checks for alpha discard, then stores normal / albedoSpec / position in the gbuffer to be used by deferred_shading
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aTexCoords;  // <vec2 position, vec2 uv scroll>
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

layout (location = 5) in mat4 instanceMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float Time;

void main() {
    vec4 worldPos = instanceMatrix * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    // apply texture UV scrolling over time  // TODO: add scrolling texture support to other shaders (tilemaps, particles)
    vs_out.TexCoords = aTexCoords.xy + (aTexCoords.zw)*Time;

    mat3 normalMatrix = transpose(inverse(mat3(instanceMatrix)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = transpose(mat3(T, B, N));

    gl_Position = projection * view * worldPos;
}