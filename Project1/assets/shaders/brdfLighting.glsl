//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoordinates;

out vec2 oTextureCoordinates;
out vec3 oEnvMapCoordinates;

uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

void main()
{	
	oTextureCoordinates = vTextureCoordinates; // Pass out texture coords
	
	// Retreive the environment map coordinates for this vertex
	vec4 unprojectedCoords = (uInverseProjection * vec4(vPosition, vec2(1.0f)));
	oEnvMapCoordinates = (uInverseView * unprojectedCoords).xyz;
	
	gl_Position = vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

in vec2 iTextureCoordinates;
in vec3 iEnvMapCoordinates;

out vec4 oColor;

const float PI = 3.14159265359f;
const float preFilterLODLevel = 4.0f;
const int MAX_LIGHTS = 100;

struct LightInfo
{
	vec3 position;
	vec3 direction;
	vec4 color;
	vec4 param1; // x = light type, y = radius, z = on/off, w = attenuationMode (0 = quadratic, 1 = UE4 style)
};

// LIGHT TYPES
// 1 = directional
// 2 = point
// 3 = IBL

// G-Buffer stuff
uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gEffects;

// Lighitng
uniform int uLightAmount;
uniform LightInfo uLightArray[MAX_LIGHTS];

// Lighting factors
uniform sampler2D uEnvMap;
uniform samplerCube uIrradianceMap;
uniform samplerCube uEnvMapPreFilter;
uniform sampler2D uEnvMapLUT;

// Extra uniforms
uniform mat4 uView;
uniform int uViewType; // 1 = color, 2 = position, 3 = normal, 4 = albedo, 5 = roughness, 6 = metalness, 7 = depth, 8 = velocity
uniform vec3 uReflectivity; // F0 https://gyazo.com/14a06a2f540467848b3e2f94cff506ad

vec3 LinearizeColor(vec3 color); // Converts to linear color space (sRGB to RGB). In other words, gamma correction https://lettier.github.io/3d-game-shaders-for-beginners/gamma-correction.html
vec2 ConvertCoordsToSpherical(vec3 normalizedCoords); // Converts a set of normalized coordinates 
float Saturate(float value); // Clamps a value from 0 to 1

// Math helpers
vec3 ComputeFresnelSchlick(float cosTheta, vec3 surfaceReflection); // Computes reflection and refraction ratio using a simplified Fresnel equation (Schlick's equation)
vec3 ComputeFresnelSchlickRoughness(float cosTheta, vec3 surfaceReflection, float roughness); // Computes reflection and refraction ratio using a simplified Fresnel equation taking roughness into consideration (Schlick's equation)
float ComputeDistibutionGGX(vec3 normal, vec3 halfwayDir, float roughness); // Computes normal distribution using GGX/Trowbridge-Reitz. Used for approximating surface area of microfacets (a piece of surface being rendered) aligned to the halfway vector
float ComputeGeometrySmith(float lightDot, float cosTheta, float roughness); // Computes surface area where micro-surface details obstruct another part of the surface causing the light ray to be occluded https://gyazo.com/ef9b603e6f1ccfc2f092563d9ca469df

// Lighting functions
void CalculatePointLight(vec3 lightPos, vec3 lightColor, float lightAttenuation, float lightRadius, vec3 viewPos, vec3 N, vec3 I, vec3 F, vec3 refractionRatio, vec3 albedo, float roughness, float cosTheta, inout vec3 color, inout vec3 diffuse, inout vec3 specular);
void CalculateDirectionalLight(vec3 lightPos, vec3 lightColor, float lightRadius, vec3 viewPos, vec3 N, vec3 I, vec3 F, vec3 refractionRatio, vec3 albedo, float roughness, float cosTheta, inout vec3 color, inout vec3 diffuse, inout vec3 specular);

void main()
{
	// Get geometry buffer data
	vec3 viewPos = texture(gPosition, iTextureCoordinates).rgb;
	vec3 albedo = LinearizeColor(texture(gAlbedo, iTextureCoordinates).rgb);
	vec3 normal = texture(gNormal, iTextureCoordinates).rgb;
	float roughness = texture(gAlbedo, iTextureCoordinates).a;
	float metalness = texture(gNormal, iTextureCoordinates).a;
	float ambientOcculsion = texture(gEffects, iTextureCoordinates).r;
	vec2 velocity = texture(gEffects, iTextureCoordinates).gb;
	float depth = texture(gPosition, iTextureCoordinates).a;
	
	vec3 environmentColor = texture(uEnvMap, ConvertCoordsToSpherical(normalize(iEnvMapCoordinates))).rgb; // Convert and sample from spherical environment map coords
	
	vec3 color = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);
	
	if(depth == 1.0f) // Nothing obstructing us here, just show the env map color
	{
		color = environmentColor;
	}
	else // We have something that we should render here, perform calculations
	{
		vec3 I = normalize(-viewPos); // Represents our view from eye - surface
		vec3 N = normalize(normal); // Represents the normal
		vec3 R = reflect(-I, N); // Reflects in respect to our view position and normal https://asawicki.info/files/Reflect_Refract.png
		float cosTheta = max(dot(N, I), 0.0001f); // Get how much force is applied in the direction of normal in relation to the view direction
		
		vec3 surfaceReflection = mix(uReflectivity, albedo, metalness); // Get the surface reflection at zero incidence (how much the surface reflects when looking directly at the surface)
		vec3 F = ComputeFresnelSchlick(cosTheta, surfaceReflection); // Approximate ratio between specular and diffuse reflection
		
		// Energy conservation https://learnopengl.com/PBR/Theory
		vec3 kS = F; // Represents energy of light that gets reflected
		vec3 refractionRatio = vec3(1.0f) - kS; // Calculate ratio of refraction
		refractionRatio *= 1.0f - metalness;
		
		for(int i = 0; i < min(uLightAmount, MAX_LIGHTS); i++)
		{
			LightInfo light = uLightArray[i];
			if(light.param1.z == 0.0f) // Light is off
			{
				continue;
			}
			
			if(light.param1.x == 1.0f) // Directional light
			{
				CalculateDirectionalLight(light.position, light.color.rgb, light.param1.y, viewPos, N, I, F, refractionRatio, albedo, roughness, cosTheta, color, diffuse, specular);
			}
			
			else if(light.param1.x == 2.0f) // Point light
			{
				CalculatePointLight(light.position, light.color.rgb, light.param1.w, light.param1.y, viewPos, N, I, F, refractionRatio, albedo, roughness, cosTheta, color, diffuse, specular);
			}
			
			else if(light.param1.x == 3.0f) // IBL light
			{
				// Re-compute reflection and energy conservation while taking roughness into consideration
				F = ComputeFresnelSchlickRoughness(cosTheta, surfaceReflection, roughness);
				
				kS = F;
				refractionRatio = vec3(1.0f) - kS;
				refractionRatio *= 1.0f - metalness;
				
				// Compute diffuse irradiance
				vec3 diffuseIrradiance = texture(uIrradianceMap, N * mat3(uView)).rgb; // Sample from irradiance cube map
				diffuseIrradiance *= albedo; // Scale by albedo
				
				// Compute specular radiance
				vec3 specularRadiance = textureLod(uEnvMapPreFilter, R * mat3(uView), roughness * preFilterLODLevel).rgb;
				vec2 brdfSampling = texture(uEnvMapLUT, vec2(cosTheta, roughness)).rg;
				specularRadiance *= F * brdfSampling.x + brdfSampling.y;
				
				// Compute the actual ambient lighting
				vec3 ambientIBL = (diffuseIrradiance * refractionRatio) + specularRadiance;
				color += ambientIBL;
			}
		}
	}
	
	// Allow viewing from a specific buffer
	if(uViewType == 1)
	{
		oColor = vec4(color, 1.0f);
	}
	else if(uViewType == 2)
	{
		oColor = vec4(viewPos, 1.0f);
	}
	else if(uViewType == 3)
	{
		oColor = vec4(normal, 1.0f);
	}
	else if(uViewType == 4)
	{
		oColor = vec4(albedo, 1.0f);
	}
	else if(uViewType == 5)
	{
		oColor = vec4(vec3(roughness), 1.0f);
	}
	else if(uViewType == 6)
	{
		oColor = vec4(vec3(metalness), 1.0f);
	}
	else if(uViewType == 7)
	{
		oColor = vec4(vec3(depth / 1000.0f), 1.0f);
	}
	else if(uViewType == 8)
	{
		oColor = vec4(velocity, 0.0f, 1.0f);
	}
}

vec3 LinearizeColor(vec3 color)
{
	return pow(color.rgb, vec3(2.2f));
}

vec2 ConvertCoordsToSpherical(vec3 normalizedCoords)
{
	float phi = acos(-normalizedCoords.y);
	float theta = atan(1.0f * normalizedCoords.x, -normalizedCoords.z) + PI;
	return vec2(theta / (2.0f * PI), phi / PI);
}

float Saturate(float value)
{
	return clamp(value, 0.0f, 1.0f);
}

vec3 ComputeFresnelSchlick(float cosTheta, vec3 surfaceReflection)
{
	return surfaceReflection + (1.0f - surfaceReflection) * pow(1.0f - cosTheta, 5.0f);
}

vec3 ComputeFresnelSchlickRoughness(float cosTheta, vec3 surfaceReflection, float roughness)
{
	return surfaceReflection + (max(vec3(1.0f - roughness), surfaceReflection) - surfaceReflection) * pow(1.0f - cosTheta, 5.0f);
}

float ComputeDistibutionGGX(vec3 normal, vec3 halfwayDir, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	
	float dottedValue = Saturate(dot(normal, halfwayDir));
	float dottedValue2 = dottedValue * dottedValue;
	
	return alpha2 / (PI * (dottedValue2 * (alpha2 - 1.0f) + 1.0f) * (dottedValue2 * (alpha2 - 1.0f) + 1.0f));
}

float ComputeGeometrySmith(float lightDot, float cosTheta, float roughness)
{
	float lightDot2 = lightDot * lightDot;
	float cosTheta2 = cosTheta * cosTheta;
	float roughness2 = roughness * roughness + 0.0001f;
	
	float ggxLight = (2.0f * lightDot) / (lightDot + sqrt(lightDot2 + roughness2 * (1.0f - lightDot2)));
	float ggxCosTheta = (2.0f * cosTheta) / (cosTheta + sqrt(cosTheta2 + roughness2 * (1.0f - cosTheta2)));
	
	return ggxLight * ggxCosTheta;
}

void CalculatePointLight(vec3 lightPos, vec3 lightColor, float lightAttenuation, float lightRadius, vec3 viewPos, vec3 N, vec3 I, vec3 F, vec3 refractionRatio, vec3 albedo, float roughness, float cosTheta, inout vec3 color, inout vec3 diffuse, inout vec3 specular)
{
	vec3 lightDir = normalize(lightPos - viewPos);
	vec3 halfwayDir = normalize(lightDir + I);
	
	vec3 gammaCorrectedColor = LinearizeColor(lightColor.rgb);
	float distance = length(lightPos - viewPos);
	
	float attenuation;
	if(lightAttenuation == 0.0f) // Quadratic atten
	{
		attenuation = 1.0f / (distance * distance);
	}
	else if(lightAttenuation == 1.0f) // UE4 atten
	{
		attenuation = pow(Saturate(1 - pow(distance / lightRadius, 4)), 2) / (distance * distance + 1);
	}
	
	float lightDot = Saturate(dot(N, lightDir)); // Get how much force is applied in the direction of normal in relation to the light direction clamped in range 0 - 1
	
	// Compute radiance
	vec3 radiance = gammaCorrectedColor * attenuation;
	
	diffuse = albedo / PI;
	
	float distribution = ComputeDistibutionGGX(N, halfwayDir, roughness);
	float geometry = ComputeGeometrySmith(lightDot, cosTheta, roughness);
	
	specular = (F * distribution * geometry) / (4.0f * lightDot * cosTheta + 0.0001f); // Compute specular component
	color += (diffuse * refractionRatio + specular) * radiance * lightDot; // Compute diffuse color
}

void CalculateDirectionalLight(vec3 lightPos, vec3 lightColor, float lightRadius, vec3 viewPos, vec3 N, vec3 I, vec3 F, vec3 refractionRatio, vec3 albedo, float roughness, float cosTheta, inout vec3 color, inout vec3 diffuse, inout vec3 specular)
{
	vec3 lightDir = normalize(lightPos - viewPos);
	vec3 halfwayDir = normalize(lightDir + I);
	
	vec3 gammaCorrectedColor = LinearizeColor(lightColor.rgb);
	
	float lightDot = Saturate(dot(N, lightDir)); // Get how much force is applied in the direction of normal in relation to the light direction clamped in range 0 - 1
	
	diffuse = albedo / PI;
	
	float distribution = ComputeDistibutionGGX(N, halfwayDir, roughness);
	float geometry = ComputeGeometrySmith(lightDot, cosTheta, roughness);
	
	specular = (F * distribution * geometry) / (4.0f * lightDot * cosTheta + 0.0001f); // Compute specular component
	color += (diffuse * refractionRatio + specular) * gammaCorrectedColor * lightDot; // Compute diffuse color
}

// RESOURCES
//https://learnopengl.com/PBR/Theory
//https://learnopengl.com/PBR/Lighting
//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
//https://learnopengl.com/PBR/IBL/Specular-IBL
//https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf