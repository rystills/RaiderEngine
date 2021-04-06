// g_buffer (fs) - checks for alpha discard, then stores normal / albedoSpec / position in the gbuffer to be used by deferred_shading
#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_emission1;

float ditherThresholdMatrix[16] = float[] (
1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
);

void main() {
    vec2 texCoords = fs_in.TexCoords;

	// discard fully transparent pixels
	float alpha = texture(texture_diffuse1, texCoords).a;
    if (alpha < .001)
        discard;

    // TODO: consider separating dither alpha from diffuse alpha to allow diffuse alpha to discard at the old threshold of 0.5 (avoids shimmering foliage due to mipmaps / averaging transparency)
    if (alpha < 1) {
        int x = int(gl_FragCoord.x) % 4;
        int y = int(gl_FragCoord.y) % 4;
        int index = x + y * 4;
        if (alpha < ditherThresholdMatrix[index])
            discard;
    }

    // store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;

	// also store the per-fragment normals into the gbuffer
	// TODO: support a variable number of maps of each type per model
	// TODO: use another shader for models that use default maps so as not to waste performance on useless calculations
	// for correct normals, make sure your crazybump settings are configured to y-axis up, x-axis right
	// if using materialize instead, you'll need to flip the x axis (r channel) in gimp
    gNormal.rgb = normalize( 2.0 * texture(texture_normal1, texCoords).rgb - vec3(1.0)) * fs_in.TBN;
	gNormal.a = texture(texture_emission1, texCoords).r;
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, texCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, texCoords).r;
}