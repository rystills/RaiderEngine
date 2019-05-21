#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;
    
	// also store the per-fragment normals into the gbuffer
	// TODO: revert to standard normal calculation if no normal map is provided
	// TODO: support a variable number of maps of each type
	// for correct normals, make sure your crazybump settings are configured to y-axis up, x-axis right
    gNormal = normalize( 2.0 * texture(texture_normal1, fs_in.TexCoords).rgb - vec3(1.0)) * fs_in.TBN;
	
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, fs_in.TexCoords).r;
}