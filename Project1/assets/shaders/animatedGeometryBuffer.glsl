//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;
layout (location = 3) in vec4 vBoneIDs;
layout (location = 4) in vec4 vBoneWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 uMatModel;
uniform mat4 uMatModelInverseTranspose;
uniform mat4 uMatView;
uniform mat4 uMatProjection;
uniform mat4 uMatProjViewModel;
uniform mat4 uMatPrevProjViewModel;
uniform vec3 uCameraPosition;
uniform mat4 uBoneMatrices[MAX_BONES];

out vec3 mWorldPosition;
out vec2 mTextureCoordinates;
out vec3 mNormal;
out vec4 mFragPosition;
out vec4 mPrevFragPosition;
out vec3 mView;

void main()
{		
	vec4 vertexPos = vec4(vPosition, 1.0f);

	vec4 transformedPos = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec3 transformedNormal = vec3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		int boneID = int(vBoneIDs[i]);
		if(boneID == -1) break; // Bone ID was still non-existent, nothing to do here
		
		if(boneID >= MAX_BONES) // We have exceeded the max bone count, just set this vertex to the default vertex position
		{
			transformedPos = vertexPos;
			transformedNormal = vNormal;
			break;
		}
		
		float weight = vBoneWeights[i];

		vec4 localPos = uBoneMatrices[boneID] * vertexPos;
		transformedPos += localPos * weight;
		
		vec3 localNormal = mat3(uBoneMatrices[boneID]) * vNormal;
		transformedNormal += localNormal * weight;
	}

	mWorldPosition = (uMatModel * transformedPos).xyz;
	
	mTextureCoordinates = vTextureCoordinates;
	
	// Apply transformation to normal
	mNormal = mat3(uMatModel) * transformedNormal;

	mFragPosition = uMatProjViewModel * transformedPos;
	mPrevFragPosition = uMatPrevProjViewModel * transformedPos;
	
	mView = normalize(mWorldPosition - uCameraPosition);
	
	gl_Position = uMatProjection * uMatView * uMatModel * transformedPos;
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
flat in ivec4 mBoneIDs;
in vec4 mWeights;

uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;
uniform vec4 uAlbedoRatios;
uniform vec4 uColorOverride;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

uniform float uShadowSoftness;

// RR = Reflectivity/Refraction
uniform samplerCube uRRMap;
uniform vec4 uRRInfo; // x =  1.0f = reflect, 2.0f = refract), y = reflectivity/refraction strength, z = refractive ratio

// Material
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

float LinearizeDepth(float depth);
vec3 ComputeTextureNormal();

void main()
{
	if(uHasNormalTexture)
	{
		gNormal.rgb = ComputeTextureNormal(); // Assign normal
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
		diffuseColor = vec3(texture(uAlbedoTexture1, mTextureCoordinates)) * uAlbedoRatios.x;
		
		if(uAlbedoRatios.y > 0.0f)
		{
			diffuseColor += vec3(texture(uAlbedoTexture2, mTextureCoordinates)) * uAlbedoRatios.y;
		}
		
		if(uAlbedoRatios.z > 0.0f)
		{
			diffuseColor += vec3(texture(uAlbedoTexture3, mTextureCoordinates)) * uAlbedoRatios.z; 
		}
		
		if(uAlbedoRatios.w > 0.0f)
		{
			diffuseColor += vec3(texture(uAlbedoTexture4, mTextureCoordinates)) * uAlbedoRatios.w; 
		}
	}
	
	if(uRRInfo.x == 1.0f) // This fragment is reflective, sample from the reflectivty map
	{
		vec3 reflectedRay = reflect(mView, mNormal); // Reflect view direction off of surface normal 
		vec3 reflectedColor = texture(uRRMap, reflectedRay).rgb; // Sample texel from cube map according to reflected ray
		diffuseColor = mix(diffuseColor, reflectedColor, uRRInfo.y); // Mix between diffuse and reflective color based on our strength [0-1]
	}
	else if(uRRInfo.x == 2.0f) // This fragment is refractive, sample from the refractive map
	{
		vec3 refractedRay = refract(mView, mNormal, uRRInfo.z); // Refract view direction off of surface normal 
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
		gAlbedo.a = vec3(texture(uRoughnessTexture, mTextureCoordinates)).r; // Sample and assign roughness value
		gNormal.a = vec3(texture(uMetalnessTexture, mTextureCoordinates)).r; // Sample and assign metalness value
		gEffects.r = vec3(texture(uAmbientOcculsionTexture, mTextureCoordinates)).r;
	}
	
	gEffects.a = uShadowSoftness;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

vec3 ComputeTextureNormal()
{
	vec3 textureNormal = normalize(texture(uNormalTexture, mTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
		
	// Get edge vectors of the pixel triangle
    vec3 dPosX = dFdx(mWorldPosition);
    vec3 dPosY = dFdy(mWorldPosition);
    vec2 dTexX = dFdx(mTextureCoordinates);
    vec2 dTexY = dFdy(mTextureCoordinates);

	// Convert normal from tangent space to world space
    vec3 normal = normalize(mNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * textureNormal);
}