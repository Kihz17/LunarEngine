//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;
layout (location = 3) in vec3 vBiNormal;
layout (location = 4) in vec3 vTangent;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;
uniform mat4 uMatProjViewModel;
uniform mat4 uMatPrevProjViewModel;

out vec3 mWorldPosition;
out vec3 mViewPosition;
out vec2 mTextureCoordinates;
out vec3 mNormal;
out vec4 mFragPosition;
out vec4 mPrevFragPosition;

void main()
{	
	// Translate to view space
	vec4 viewFragmentPosition = uMatView * uMatModel * vec4(vPosition, 1.0f);
	mViewPosition = viewFragmentPosition.xyz;
	
	mWorldPosition = vec3(uMatModel * vec4(vPosition, 1.0f));
	
	mTextureCoordinates = vTextureCoordinates;
	
	// Apply transformation to normal
	mat3 matNormal = transpose(inverse(mat3(uMatView * uMatModel)));
	mNormal = matNormal * vNormal;
	
	mFragPosition = uMatProjViewModel * vec4(vPosition, 1.0f);
	mPrevFragPosition = uMatPrevProjViewModel * vec4(vPosition, 1.0f);
	
	gl_Position = uMatProjection * viewFragmentPosition;
};



//type fragment
#version 420 

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec3 gEffects;
layout (location = 4) out vec3 gViewPosition;

in vec3 mWorldPosition;
in vec3 mViewPosition;
in vec2 mTextureCoordinates;
in vec3 mNormal;
in vec4 mFragPosition;
in vec4 mPrevFragPosition;

uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;
uniform vec4 uAlbedoRatios;

uniform sampler2D uNormalTexture;
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, a = isMaterialOverride

const float nearPlane = 1.0f;
const float farPlane = 1000.0f;

float LinearizeDepth(float depth);
vec3 ComputeTextureNormal(vec3 viewNormal, vec3 textureNormal);

void main()
{
	vec3 normal = normalize(texture(uNormalTexture, mTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
	
	vec2 fragPos = (mFragPosition.xy / mFragPosition.w) * 0.5f + 0.5f;
	vec2 prevFragPos = (mPrevFragPosition.xy / mPrevFragPosition.w) * 0.5f + 0.5f;
	
	gPosition = vec4(mWorldPosition, LinearizeDepth(gl_FragCoord.z)); // Set position with adjusted depth
	gViewPosition = mViewPosition;
	
	gAlbedo.rgb = vec3(texture(uAlbedoTexture1, mTextureCoordinates)) * uAlbedoRatios.x; // Sample and assign albedo rgb colors
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

	gNormal.rgb = ComputeTextureNormal(mNormal, normal); // Assign normal

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