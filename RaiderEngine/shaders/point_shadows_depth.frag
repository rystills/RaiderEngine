// point_shadows_depth (fs) - renders point light shadows to a depth cubemap
#version 330 core
in vec4 FragPos;
in vec2 texCoords;

uniform vec3 lightPos;
uniform float far_plane;
uniform sampler2D texture_diffuse1;

void main()
{
    // opacity check
    vec4 textureColour = texture(texture_diffuse1, texCoords);
    // treat semitransparent pixels as solid
    if (textureColour.a < 0.001) {
        discard;
    }
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}