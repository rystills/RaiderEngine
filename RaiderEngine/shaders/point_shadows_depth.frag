// point_shadows_depth (fs) - renders point light shadows to a depth cubemap
#version 330 core
in vec4 FragPos;
in vec2 texCoords;

uniform vec3 lightPos;
uniform float far_plane;
uniform sampler2D texture_diffuse1;

// NOTE: matrix and discard lookup duplicated from g_buffer.frag
float ditherThresholdMatrix[16] = float[] (
1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
);

void main()
{
    // opacity check
    vec4 textureColour = texture(texture_diffuse1, texCoords);
    // treat semitransparent pixels as solid
    if (textureColour.a < .001) {
        discard;
    }

    if (textureColour.a < 1) {
        int x = int(gl_FragCoord.x) % 4;
        int y = int(gl_FragCoord.y) % 4;
        int index = x + y * 4;
        if (textureColour.a < ditherThresholdMatrix[index])
            discard;
    }

    float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}