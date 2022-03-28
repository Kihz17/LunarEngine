//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;

uniform mat4 uMatModel;
uniform mat4 uMatModelInverseTranspose;
uniform mat4 uMatView;
uniform mat4 uMatProjection;
uniform mat4 uMatProjViewModel;
uniform mat4 uMatPrevProjViewModel;
uniform vec3 uCameraPosition;

out vec3 mWorldPosition;
out vec2 mTextureCoordinates;
out vec3 mNormal;
out vec4 mFragPosition;
out vec4 mPrevFragPosition;
out vec3 mView;

void main()
{		
	vec4 vertexPos = vec4(vPosition, 1.0f);
	mWorldPosition = (uMatModel * vertexPos).xyz;
	
	mTextureCoordinates = vTextureCoordinates;
	
	// Apply transformation to normal
	mNormal = mat3(uMatModel) * vNormal;

	mFragPosition = uMatProjViewModel * vertexPos;
	mPrevFragPosition = uMatPrevProjViewModel * vertexPos;
	
	mView = normalize(mWorldPosition - uCameraPosition);
	
	gl_Position = uMatProjection * uMatView * uMatModel * vertexPos;
};



//type fragment
#version 420 

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gEffects;

in vec3 mWorldPosition;
in vec2 mTextureCoordinates;
in vec3 mNormal;
in vec4 mFragPosition;
in vec4 mPrevFragPosition;
in vec3 mView;

uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;
uniform vec4 uAlbedoRatios;
uniform vec2 uUVOffset;
uniform vec4 uColorOverride;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

uniform float uShadowSoftness;

// RR = Reflectivity/Refraction
uniform samplerCube uRRMap;
uniform vec4 uRRInfo; // x =  1.0f = reflect, 2.0f = refract), y = reflectivity/refraction strength, z = refractive ratio

// Material
uniform sampler2D uORMTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

float LinearizeDepth(float depth);
vec3 ComputeTextureNormal(vec2 uv);

void main()
{
	vec2 texCoords = mTextureCoordinates * uUVOffset;

	if(uHasNormalTexture)
	{
		gNormal.rgb = ComputeTextureNormal(texCoords); // Assign normal
	}
	else
	{
		gNormal.rgb = mNormal;
	}
	
	vec2 fragPos = (mFragPosition.xy / mFragPosition.w) * 0.5f + 0.5f;
	vec2 prevFragPos = (mPrevFragPosition.xy / mPrevFragPosition.w) * 0.5f + 0.5f;
	
	gPosition = vec4(mWorldPosition, LinearizeDepth(gl_FragCoord.z)); // Set position with adjusted depth

	vec3 diffuseColor = vec3(0.0f);
	if(uColorOverride.w == 1.0f) // Override color
	{
		diffuseColor = uColorOverride.rgb;
	}
	else // Sample albedo textures
	{
		diffuseColor = vec3(texture(uAlbedoTexture1, texCoords)) * uAlbedoRatios.x;
		
		if(uAlbedoRatios.y > 0.0f)
		{
			diffuseColor += vec3(texture(uAlbedoTexture2, texCoords)) * uAlbedoRatios.y;
		}
		
		if(uAlbedoRatios.z > 0.0f)
		{
			diffuseColor += vec3(texture(uAlbedoTexture3, texCoords)) * uAlbedoRatios.z; 
		}
		
		if(uAlbedoRatios.w > 0.0f)
		{
			diffuseColor += vec3(texture(uAlbedoTexture4, texCoords)) * uAlbedoRatios.w; 
		}
	}
	
	if(uRRInfo.x == 1.0f) // This fragment is reflective, sample from the reflectivty map
	{
		vec3 reflectedRay = reflect(mView, gNormal.rgb); // Reflect view direction off of surface normal 
		vec3 reflectedColor = texture(uRRMap, reflectedRay).rgb; // Sample texel from cube map according to reflected ray
		diffuseColor = mix(diffuseColor, reflectedColor, uRRInfo.y); // Mix between diffuse and reflective color based on our strength [0-1]
	}
	else if(uRRInfo.x == 2.0f) // This fragment is refractive, sample from the refractive map
	{
		vec3 refractedRay = refract(mView, gNormal.rgb, uRRInfo.z); // Refract view direction off of surface normal 
		vec3 refractedColor = texture(uRRMap, refractedRay).rgb; // Sample texel from cube map according to refracted ray
		diffuseColor = mix(diffuseColor, refractedColor, uRRInfo.y); // Mix between diffuse and refracted color based on our strength [0-1]
	}
	
	gAlbedo.rgb = diffuseColor;

	gEffects.gb = fragPos - prevFragPos;
	
	if(uMaterialOverrides.w == 1.0f) // No texture to sample from, use flat value
	{
		gAlbedo.a = uMaterialOverrides.r; // Roughness
		gNormal.a = uMaterialOverrides.g; // Metalness
		gEffects.r = uMaterialOverrides.b; // AO
	}
	else
	{
		vec4 ormSample = texture(uORMTexture, texCoords);
		gAlbedo.a = ormSample.g; // Sample and assign roughness value
		gNormal.a = ormSample.b; // Sample and assign metalness value
		gEffects.r = ormSample.r;
	}
	
	gEffects.a = uShadowSoftness;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

vec3 ComputeTextureNormal(vec2 uv)
{
	vec3 textureNormal = normalize(texture(uNormalTexture, uv).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
		
	// Get edge vectors of the pixel triangle
    vec3 dPosX = dFdx(mWorldPosition);
    vec3 dPosY = dFdy(mWorldPosition);
    vec2 dTexX = dFdx(uv);
    vec2 dTexY = dFdy(uv);

	// Convert normal from tangent space to world space
    vec3 normal = normalize(mNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * textureNormal);
}