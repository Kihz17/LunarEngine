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

out vec3 mWorldPosition;
out vec2 mTextureCoordinates;
out vec3 mNormal;
out vec4 mFragPosition;
out vec4 mPrevFragPosition;

void main()
{		
	vec4 vertexPos = vec4(vPosition, 1.0f);
	mWorldPosition = (uMatModel * vertexPos).xyz;
	
	mTextureCoordinates = vTextureCoordinates;
	
	// Apply transformation to normal
	mNormal = mat3(uMatModel) * vNormal;

	mFragPosition = uMatProjViewModel * vertexPos;
	mPrevFragPosition = uMatPrevProjViewModel * vertexPos;
	
	gl_Position = uMatProjection * uMatView * uMatModel * vertexPos;
};



//type fragment
#version 420 

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec3 gEffects;

in vec3 mWorldPosition;
in vec2 mTextureCoordinates;
in vec3 mNormal;
in vec4 mFragPosition;
in vec4 mPrevFragPosition;

uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;
uniform vec4 uAlbedoRatios;
uniform vec4 uColorOverride;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

// Material
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

const float nearPlane = 1.0f;
const float farPlane = 1000.0f;

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
	
	if(uColorOverride.w == 1.0f) // Override color
	{
		gAlbedo.rgb = uColorOverride.rgb;
	}
	else // Sample albedo textures
	{
		gAlbedo.rgb = vec3(texture(uAlbedoTexture1, mTextureCoordinates)) * uAlbedoRatios.x;
		
		if(uAlbedoRatios.y > 0.0f)
		{
			gAlbedo.rgb += vec3(texture(uAlbedoTexture2, mTextureCoordinates)) * uAlbedoRatios.y;
		}
		
		if(uAlbedoRatios.z > 0.0f)
		{
			gAlbedo.rgb += vec3(texture(uAlbedoTexture3, mTextureCoordinates)) * uAlbedoRatios.z; 
		}
		
		if(uAlbedoRatios.w > 0.0f)
		{
			gAlbedo.rgb += vec3(texture(uAlbedoTexture4, mTextureCoordinates)) * uAlbedoRatios.w; 
		}
	}

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
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
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