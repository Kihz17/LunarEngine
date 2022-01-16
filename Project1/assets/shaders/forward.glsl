//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;

out vec3 mNormal;
out vec3 mVertexNormal;
out vec2 mTextureCoordinates;
out vec3 mWorldPosition;
out vec3 mViewPosition;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;
uniform mat4 uMatModelInverseTranspose;

void main()
{	
	// Translate to view space
	vec4 viewFragmentPosition = uMatView * uMatModel * vec4(vPosition, 1.0f);
	mViewPosition = viewFragmentPosition.xyz;
	
	mTextureCoordinates = vTextureCoordinates;
	
	// Apply transformation to normal
	mat3 matNormal = transpose(inverse(mat3(uMatView * uMatModel)));
	mNormal = matNormal * vNormal;
	
	// Calculate the normal based on any rotation we've applied.
	// This inverse transpose removes scaling and translation (movement) 
	// 	from the matrix.
	mVertexNormal = vec3(uMatModelInverseTranspose * normalize(vec4(vNormal, 1.0f)));
	mVertexNormal = normalize(mNormal);
	
	mWorldPosition = vec3(uMatModel * vec4(vPosition, 1.0f));
	gl_Position = uMatProjection * uMatView * uMatModel * vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

out vec3 mNormal;
in vec3 mVertexNormal;
in vec2 mTextureCoordinates;
in vec3 mWorldPosition;
in vec3 mViewPosition;

out vec4 oColor;

const float PI = 3.14159265359f;
const float preFilterLODLevel = 4.0f;
const int MAX_LIGHTS = 100;
const float ATTEN_MULT = 100.0f;

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

uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;
uniform vec4 uAlbedoRatios;
uniform vec4 uColorOverride;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

uniform float uAlphaTransparency;
uniform bool uIgnoreLighting;

// Lighitng
uniform int uLightAmount;
uniform LightInfo uLightArray[MAX_LIGHTS];

uniform vec3 uReflectivity; 

vec3 ComputeTextureNormal(vec3 viewNormal, vec3 textureNormal);

vec3 LinearizeColor(vec3 color); // Converts to linear color space (sRGB to RGB). In other words, gamma correction https://lettier.github.io/3d-game-shaders-for-beginners/gamma-correction.html
vec2 ConvertCoordsToSpherical(vec3 normalizedCoords); // Converts a set of normalized coordinates 
float Saturate(float value); // Clamps a value from 0 to 1

// Math helpers
vec3 ComputeFresnelSchlick(float cosTheta, vec3 surfaceReflection); // Computes reflection and refraction ratio using a simplified Fresnel equation (Schlick's equation)
vec3 ComputeFresnelSchlickRoughness(float cosTheta, vec3 surfaceReflection, float roughness); // Computes reflection and refraction ratio using a simplified Fresnel equation taking roughness into consideration (Schlick's equation)
float ComputeDistibutionGGX(vec3 normal, vec3 halfwayDir, float roughness); // Computes normal distribution using GGX/Trowbridge-Reitz. Used for approximating surface area of microfacets (a piece of surface being rendered) aligned to the halfway vector
float ComputeGeometrySmith(float lightDot, float cosTheta, float roughness); // Computes surface area where micro-surface details obstruct another part of the surface causing the light ray to be occluded https://gyazo.com/ef9b603e6f1ccfc2f092563d9ca469df
float ComputeGeometrySchlickGGX(float NdotV, float roughness);

void main()
{
	vec3 worldPos = mWorldPosition;
	vec3 viewPos = mViewPosition;
	
	vec3 albedo;
	if(uColorOverride.w == 1.0f) // Override color
	{
		albedo.rgb = uColorOverride.rgb;
	}
	else
	{
		albedo.rgb += texture(uAlbedoTexture1, mTextureCoordinates).rgb * uAlbedoRatios.x;
		albedo.rgb += texture(uAlbedoTexture2, mTextureCoordinates).rgb * uAlbedoRatios.y;
		albedo.rgb += texture(uAlbedoTexture3, mTextureCoordinates).rgb * uAlbedoRatios.z;
		albedo.rgb += texture(uAlbedoTexture4, mTextureCoordinates).rgb * uAlbedoRatios.w;
		albedo = LinearizeColor(albedo);
	}
	
	vec3 normal;
	if(uHasNormalTexture)
	{
		vec3 N = normalize(texture(uNormalTexture, mTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
		normal = ComputeTextureNormal(mNormal, N); // Assign normal
	}
	else
	{
		normal = mVertexNormal;
	}
	
	float roughness;
	float metalness;
	float ambientOcculsion;
	if(uMaterialOverrides.w == 1.0f) // No texture to sample from, use flat value
	{
		roughness = uMaterialOverrides.r;
		metalness = uMaterialOverrides.g;
		ambientOcculsion = uMaterialOverrides.b;
	}
	else
	{
		roughness = vec3(texture(uRoughnessTexture, mTextureCoordinates)).r; // Sample and assign roughness value
		metalness = vec3(texture(uMetalnessTexture, mTextureCoordinates)).r; // Sample and assign metalness value
		ambientOcculsion = vec3(texture(uAmbientOcculsionTexture, mTextureCoordinates)).r;
	}
	
	vec3 color = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);
	
	if(uIgnoreLighting)
	{
		color = albedo;
	}
	else
	{
		vec3 V = normalize(-viewPos); // Represents our view from eye - surface
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
				
				vec3 gammaCorrectedColor = LinearizeColor(light.color.rgb);
				
				float lightDot = Saturate(dot(N, lightDir)); // Get how much force is applied in the direction of normal in relation to the light direction clamped in range 0 - 1
				
				diffuse = albedo / PI;
				
				float distribution = ComputeDistibutionGGX(N, halfwayDir, roughness);
				float geometry = ComputeGeometrySmith(lightDot, cosTheta, roughness);
				
				float HdotI = Saturate(dot(halfwayDir, V));
				vec3 lightFresnel = ComputeFresnelSchlick(HdotI, surfaceReflection); // Approximate ratio between specular and diffuse reflection
				specular = (lightFresnel * distribution * geometry) / (4.0f * lightDot * cosTheta + 0.0001f); // Compute specular component
				
				// Energy conservation
				vec3 kS = lightFresnel; // Represents energy of light that gets reflected
				vec3 kD = vec3(1.0f) - kS;
				kD *= 1.0f - metalness;
				
				color += (kD * diffuse + specular) * gammaCorrectedColor * lightDot; // Compute diffuse color
			}
			
			else if(light.param1.x == 1.0f) // Point light
			{	
				vec3 lightDir = normalize(light.position - worldPos);
				vec3 halfwayDir = normalize(lightDir + V);

				vec3 gammaCorrectedColor = LinearizeColor(light.color.rgb);
				float distance = length(light.position - worldPos);

				float attenuation;
				if(light.param1.w == 0.0f) // Linear atten
				{
					attenuation = 1.0f / distance * 4.0f;
				}
				if(light.param1.w == 1.0f) // Quadratic atten
				{
					attenuation = 1.0f / (distance * distance);
				}
				else if(light.param1.w == 2.0f) // UE4 atten
				{
					attenuation = pow(Saturate(1 - pow(distance / light.param1.y, 4)), 2) / (distance * distance + 1);
				}
				
				attenuation *= ATTEN_MULT;

				// Compute radiance
				vec3 radiance = gammaCorrectedColor * attenuation;
				
				float lightDot = Saturate(dot(N, lightDir)); // Get how much force is applied in the direction of normal in relation to the light direction clamped in range 0 - 1
						
				float distribution = ComputeDistibutionGGX(N, halfwayDir, roughness);
				float geometry = ComputeGeometrySmith(lightDot, cosTheta, roughness);
				
				float HdotI = Saturate(dot(halfwayDir, V));
				vec3 lightFresnel = ComputeFresnelSchlick(HdotI, surfaceReflection); // Approximate ratio between specular and diffuse reflection
				specular = (lightFresnel * distribution * geometry) / (4.0f * lightDot * cosTheta + 0.0001f); // Compute specular component
				
				// Energy conservation
				vec3 kS = lightFresnel; // Represents energy of light that gets reflected
				vec3 kD = vec3(1.0f) - kS;
				kD *= 1.0f - metalness;
				
				diffuse = albedo / PI;
				color += (kD * diffuse + specular) * radiance * lightDot; // Compute diffuse color
			}
		}
	}
		
	oColor = vec4(color, uAlphaTransparency);
}

vec3 ComputeTextureNormal(vec3 viewNormal, vec3 textureNormal)
{
	// Get partial derivatives 
    vec3 dPosX = dFdx(mViewPosition);
    vec3 dPosY = dFdy(mViewPosition);
    vec2 dTexX = dFdx(mTextureCoordinates);
    vec2 dTexY = dFdy(mTextureCoordinates);

	// Convert normal to tangent space
    vec3 normal = normalize(viewNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * textureNormal);
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
	float ggx2 = ComputeGeometrySchlickGGX(cosTheta, roughness);
	float ggx1 = ComputeGeometrySchlickGGX(lightDot, roughness);
	return ggx1 * ggx2;
}

float ComputeGeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}