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

#define numPoisson 16
vec2 poissonDisk[numPoisson] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

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
    for (int i = 0; i < 20; ++i) { 
        int index = int(numPoisson*random(floor(fragPos*1000.0), i))%numPoisson;
        // use the fragment to light vector to sample from the depth map
        // TODO: 3d poisson sample rather than using the x value of the next entry for the 3rd dimension
        float PCF = texture(lightNum == 0 ? depthMap0 : (lightNum == 1 ? depthMap1 : (lightNum == 2 ? depthMap2 : (lightNum == 3 ? depthMap3 : depthMap4))), 
        vec4(fragToLight + vec3(poissonDisk[index],poissonDisk[(index+1)%numPoisson].x) * diskRadius,currentDepth/far_plane-bias));
        shadow += 1-PCF;
        // NOTE: early bail test (bailing after more iterations improves edge detection but reduces performance)
        if (i == 3 && (shadow == 0 || shadow == 4))
            return shadow/4.0;
    }
    return shadow / float(20);
        
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
