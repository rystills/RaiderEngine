// deferred_shading (fs) - heavy lifting shader; retrives the data from the gbuffer, then calculates lighting, shadows, etc.
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// shadows
uniform samplerCubeShadow depthMap0;
uniform samplerCubeShadow depthMap1;
uniform samplerCubeShadow depthMap2;
uniform samplerCubeShadow depthMap3;
uniform samplerCubeShadow depthMap4;
uniform float far_plane;
uniform float ambientStrength = 0;
uniform vec4 clearColor;

struct Light {
    vec3 Position;
    vec3 Color;
    bool On;
    float Linear;
    float Quadratic;
    float Radius;
};
// TODO: allow variable # of lights rather than fixed 5 light maximum
uniform int NR_LIGHTS;
uniform Light lights[5]; // hard-coded maximum of 5 lights for now
uniform vec3 viewPos;

#define numSamples 20
// TODO: should these points be normalized? currently the first 8 points (the cube vertices) are sampling farther away than the remaining 12 (this might actually be a good thing for early bail detection)
vec3 gridSamplingDisk[numSamples] = vec3[] (
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
// NOTE: 8 samples chosen for early bail as the first 8 entries in the grid sampling disk represent the vertices of a cube, resulting in an effectively distributed initial sampling
#define earlyBailCount 8

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float ShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightPos, int lightNum) {
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
	vec3 lightDir = normalize(lightPos - fragPos);
    // NOTE: decreasing the bias range results in less peter panning but more shadow acne (tweaked from .05-.005 to .15-.03, divided by far_plane for samplerCubeShadow)
    float bias = max(0.15/far_plane * (1.0 - dot(normal, lightDir)), .03/far_plane);
    float viewDistance = length(viewPos - fragPos);
    // NOTE: increasing the divisor results in harder shadows (tweaked from 25 to 50)
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 50.0;
    float shadow = 0;
    for (int i = 0; i < numSamples; ++i) { 
        // use the fragment to light vector to sample from the depth map
        shadow += 1-texture(lightNum == 0 ? depthMap0 : (lightNum == 1 ? depthMap1 : (lightNum == 2 ? depthMap2 : (lightNum == 3 ? depthMap3 : depthMap4))), 
        vec4(fragToLight + gridSamplingDisk[i] * diskRadius,currentDepth/far_plane-bias));
        // NOTE: early bail test (bailing after more iterations improves edge detection but reduces performance)
        if (i == earlyBailCount-1 && (shadow == 0 || shadow == earlyBailCount))
            return shadow/earlyBailCount;
    }
    return shadow / numSamples;
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
}

void main()
{             
    // retrieve data from gbuffer
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    // respect the existing clear color if we have nothing to draw
    if (Normal == vec3(0.0, 0.0, 0.0)) { 
        discard; 
        // NOTE: we can avoid setting clearColor to black and then back to the desired color each frame by 
        // having it always be black and setting the desired color here instead via FragColor = clearColor * ambientStrength;
        // however, that would disallow rendering in front of something other than a solid color (ie. 2D sprite/skybox) so it is not used
    }
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    float emission = texture(gNormal, TexCoords).a;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 ambient = Diffuse * ambientStrength;
	vec3 diffuseSpec = vec3(0,0,0);
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i) {
		if (lights[i].On) {
			// calculate distance between light source and current fragment
			float distance = length(lights[i].Position - FragPos);
			if(distance < lights[i].Radius) {
				float shadow = ShadowCalculation(FragPos, Normal, lights[i].Position, i);
				// diffuse
				vec3 lightDir = normalize(lights[i].Position - FragPos);
				vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
				// specular
				vec3 halfwayDir = normalize(lightDir + viewDir);  
				float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
				vec3 specular = lights[i].Color * spec * Specular;
				// attenuation
				float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
				diffuse *= attenuation;
				specular *= attenuation;
				diffuseSpec += (1-shadow)*(diffuse + specular);
			}
		}
    }    
	vec3 lighting = ambient + diffuseSpec + emission;// * Diffuse;   
	// apply linear fog matching clear color, starting at 10 units and capping out at 50 units
	// formula: min(1,-x + y*max(minval,dist)) where y = 1/(maxval-minval), x = -y*minval
	float fog = min(1,-.25+.025*max(10,distance(viewPos,FragPos)));
	lighting = mix(lighting,clearColor.xyz*ambientStrength,fog);
    FragColor = vec4(lighting, 1.0);
}
