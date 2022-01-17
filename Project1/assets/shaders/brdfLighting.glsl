//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoordinates;

out vec2 mTextureCoordinates;
out vec3 mEnvMapCoordinates;
out mat4 mInverseView;
out mat4 mInverseProjection;

uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

void main()
{	
	mTextureCoordinates = vTextureCoordinates; // Pass out texture coords
	
	// Retreive the environment map coordinates for this vertex
	vec4 unprojectedCoords = (uInverseProjection * vec4(vPosition, vec2(1.0f)));
	mEnvMapCoordinates = (uInverseView * unprojectedCoords).xyz;
	
	gl_Position = vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

in vec2 mTextureCoordinates;
in vec3 mEnvMapCoordinates;
in mat4 mInverseView;
in mat4 mInverseProjection;

out vec4 oColor;

const float PI = 3.14159265359f;
const float preFilterLODLevel = 4.0f;
const int MAX_LIGHTS = 100;
const float ATTEN_MULT = 10.0f;

struct LightInfo
{
	vec3 position;
	vec3 direction;
	vec4 color; // a = intensity
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
uniform vec4 gMaterialOverrides;

// Lighitng
uniform int uLightAmount;
uniform LightInfo uLightArray[MAX_LIGHTS];

// Lighting factors
uniform sampler2D uEnvMap;
uniform samplerCube uIrradianceMap;
uniform samplerCube uEnvMapPreFilter;
uniform sampler2D uEnvMapLUT;

// Extra uniforms
uniform vec3 uCameraPosition;
uniform mat4 uView;
uniform int uViewType; // 1 = color, 2 = position, 3 = normal, 4 = albedo, 5 = roughness, 6 = metalness, 7 = depth, 8 = velocity
uniform vec3 uReflectivity; // F0 https://gyazo.com/14a06a2f540467848b3e2f94cff506ad

vec3 LinearizeColor(vec3 color); // Converts to linear color space (sRGB to RGB). In other words, gamma correction https://lettier.github.io/3d-game-shaders-for-beginners/gamma-correction.html
float Saturate(float value); // Clamps a value from 0 to 1

// Math helpers
vec3 ComputeFresnelSchlick(float cosTheta, vec3 surfaceReflection); // Computes reflection and refraction ratio using a simplified Fresnel equation (Schlick's equation)
vec3 ComputeFresnelSchlickRoughness(float cosTheta, vec3 surfaceReflection, float roughness); // Computes reflection and refraction ratio using a simplified Fresnel equation taking roughness into consideration (Schlick's equation)
float ComputeDistibutionGGX(vec3 normal, vec3 halfwayDir, float roughness); // Computes normal distribution using GGX/Trowbridge-Reitz. Used for approximating surface area of microfacets aligned to the halfway vector (determines strength & area for specular light)
float ComputeGeometrySmith(float lightDot, float cosTheta, float roughness); // Computes surface area where micro-surface details obstruct another part of the surface causing the light ray to be occluded https://gyazo.com/ef9b603e6f1ccfc2f092563d9ca469df
float ComputeGeometrySchlickGGX(float NdotV, float roughness);

void main()
{
	// Get geometry buffer data
	vec3 worldPos = texture(gPosition, mTextureCoordinates).rgb;
	vec3 albedo = LinearizeColor(texture(gAlbedo, mTextureCoordinates).rgb);
	vec3 normal = texture(gNormal, mTextureCoordinates).rgb;
	float roughness = texture(gAlbedo, mTextureCoordinates).a;
	float metalness = texture(gNormal, mTextureCoordinates).a;
	float ambientOcculsion = texture(gEffects, mTextureCoordinates).r;
	vec2 velocity = texture(gEffects, mTextureCoordinates).gb;
	float depth = texture(gPosition, mTextureCoordinates).a;
	
	vec3 color = vec3(0.0f);
		
	if(depth == 1.0f) // Nothing obstructing us here, just show the env map color
	{
		color = texture(uEnvMap, mTextureCoordinates).rgb; // Convert and sample from spherical environment map coords
	}
	else // We have something that we should render here, perform calculations
	{
		//color = (albedo * 0.1f) / PI; // Show some dark color at the very least
		
		vec3 V = normalize(uCameraPosition - worldPos); // Represents our view from eye - surface
		vec3 N = normalize(normal); // Represents the normal
		vec3 R = reflect(-V, N); // Reflects in respect to our view position and normal https://asawicki.info/files/Reflect_Refract.png
		float cosTheta = max(dot(N, V), 0.0001f); // Get how much force is applied in the direction of normal in relation to the view direction
		
		vec3 surfaceReflection = mix(uReflectivity, albedo, metalness); // Get the surface reflection at zero incidence (how much the surface reflects when looking directly at the surface)

		for(int i = 0; i < min(uLightAmount, MAX_LIGHTS); i++)
		{
			LightInfo light = uLightArray[i];
			if(light.param1.z == 0.0f) // Light is off
			{
				continue;
			}
			
			if(length(worldPos - light.position) > light.param1.y) // Outside of reach
			{
				continue;
			}
			
			if(light.param1.x == 0.0f) // Directional light
			{	
				vec3 lightDir = normalize(-light.direction);
				vec3 halfwayDir = normalize(lightDir + V);
				
				float lightDot = Saturate(dot(N, lightDir)); // Get how much force is applied in the direction of normal in relation to the light direction clamped in range 0 - 1
				
				float distribution = ComputeDistibutionGGX(N, halfwayDir, roughness);
				float geometry = ComputeGeometrySmith(lightDot, cosTheta, roughness);
				
				float HdotI = Saturate(dot(halfwayDir, V));
				vec3 lightFresnel = ComputeFresnelSchlick(HdotI, surfaceReflection); // Approximate ratio between specular and diffuse reflection
				vec3 specular = (lightFresnel * distribution * geometry) / (4.0f * lightDot * cosTheta + 0.0001f); // Compute specular component
				
				// Energy conservation
				vec3 kS = lightFresnel; // Represents specular ratio
				vec3 kD = vec3(1.0f) - kS; // Represents diffuse ratio
				kD *= 1.0f - metalness;
				
				vec3 diffuse = kD * albedo / PI;
				color += (diffuse + specular) * light.color.rgb * lightDot; // Compute diffuse color
			}
			
			else if(light.param1.x == 1.0f) // Point light
			{	
				vec3 lightDir = light.position - worldPos;
				float distance = length(lightDir);
				lightDir = normalize(lightDir);
				vec3 halfwayDir = normalize(lightDir + V);

				float attenuation;
				if(light.param1.w == 0.0f) // Linear atten
				{
					attenuation = 1.0f / distance;
				}
				else if(light.param1.w == 1.0f) // Quadratic atten
				{
					attenuation = 1.0f / (distance * distance);
				}
				else if(light.param1.w == 2.0f) // UE4 atten
				{
					float d = distance / light.param1.y;
					float d2 = d * d;
					float d4 = d2 * d2;
					float fallOff = Saturate(1.0f - d4);
					attenuation = (fallOff * fallOff) / ((distance * distance) + 1.0f);
				}

				// Compute radiance
				vec3 radiance = light.color.rgb * attenuation * light.color.a;
				
				float lightDot = Saturate(dot(N, lightDir)); // Get how much force is applied in the direction of normal in relation to the light direction clamped in range 0 - 1
						
				// Cook-Torrance BRDF calculations
				float distribution = ComputeDistibutionGGX(N, halfwayDir, roughness);
				float geometry = ComputeGeometrySmith(lightDot, cosTheta, roughness);
				vec3 lightFresnel = ComputeFresnelSchlick(Saturate(dot(halfwayDir, V)), surfaceReflection); // Approximate ratio between specular and diffuse reflection
				
				// Energy conservation
				vec3 kS = lightFresnel; // Represents specular ratio
				vec3 kD = vec3(1.0f) - kS; // Represents diffuse ratio
				kD *= 1.0f - metalness;
				
				// Compute specular component (Cook-Torrance BRDF)
				vec3 specular = (kS * distribution * geometry) / (4.0f * lightDot * cosTheta + 0.0001f); 
								
				vec3 diffuse = kD * albedo / PI;
				color += (diffuse + specular) * radiance * lightDot; // Compute diffuse color
			}
			
			else if(light.param1.x == 2.0f) // IBL light
			{
				// Re-compute reflection and energy conservation while taking roughness into consideration
				vec3 F = ComputeFresnelSchlickRoughness(cosTheta, surfaceReflection, roughness);  // Approximate ratio between specular and diffuse reflection
				
				// Energy conservation
				vec3 kS = F;
				vec3 kD = vec3(1.0f) - kS;
				kD *= 1.0f - metalness;
				
				// Compute diffuse irradiance
				vec3 diffuseIrradiance = texture(uIrradianceMap, N * mat3(uView)).rgb; // Sample from irradiance cube map
				diffuseIrradiance *= albedo; // Scale by albedo
				
				// Compute specular radiance
				vec3 specularRadiance = textureLod(uEnvMapPreFilter, R * mat3(uView), roughness * preFilterLODLevel).rgb;
				vec2 brdfSampling = texture(uEnvMapLUT, vec2(cosTheta, roughness)).rg;
				specularRadiance *= F * brdfSampling.x + brdfSampling.y;
				
				// Compute the actual ambient lighting
				vec3 ambientIBL = (diffuseIrradiance * kD) + specularRadiance;
				color += ambientIBL;
			}
		}
	}
	
	// Gamma correction
	color.rgb = LinearizeColor(color.rgb);
	
	// Allow viewing from a specific buffer
	if(uViewType == 1)
	{
		oColor = vec4(color, 1.0f);
	}
	else if(uViewType == 2)
	{
		oColor = vec4(worldPos, 1.0f);
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

float Saturate(float value)
{
	return clamp(value, 0.0f, 1.0f);
}

vec3 ComputeFresnelSchlick(float cosTheta, vec3 surfaceReflection)
{
	return surfaceReflection + (1.0f - surfaceReflection) * pow(2.0f, (-5.55473 * cosTheta - 6.98316) * cosTheta);
}

vec3 ComputeFresnelSchlickRoughness(float cosTheta, vec3 surfaceReflection, float roughness)
{
	return surfaceReflection + (max(vec3(1.0f - roughness), surfaceReflection) - surfaceReflection) * pow(1.0f - cosTheta, 5.0f);
}

float ComputeDistibutionGGX(vec3 normal, vec3 halfwayDir, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	
	float NdotH = Saturate(dot(normal, halfwayDir));
	float NdotH2 = NdotH * NdotH;
	
	float divisor = (NdotH2 * (alpha2 - 1.0f) + 1.0f);
	divisor = PI * divisor * divisor;
	
	return alpha2 / divisor;
}

float ComputeGeometrySmith(float lightDot, float cosTheta, float roughness)
{
	float ggx2 = ComputeGeometrySchlickGGX(cosTheta, roughness);
	float ggx1 = ComputeGeometrySchlickGGX(lightDot, roughness);
	return ggx1 * ggx2;
}

float ComputeGeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
 
    float divisor = NdotV * (1.0f - k) + k;

    return NdotV / max(divisor, 0.001f);
}

// RESOURCES
//https://learnopengl.com/PBR/Theory
//https://learnopengl.com/PBR/Lighting
//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
//https://learnopengl.com/PBR/IBL/Specular-IBL
//https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf