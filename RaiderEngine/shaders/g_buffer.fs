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

void main() {
    vec2 texCoords = fs_in.TexCoords;
    
	// discard transparent pixels
	if(texture(texture_diffuse1, texCoords).a < .5)
        discard;

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