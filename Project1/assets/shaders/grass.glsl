//type vertex
#version 420

layout (location = 0) in vec3 vWorldPosition;

void main()
{	
	gl_Position = vec4(vWorldPosition, 1.0f);
};

//type geometry
#version 420

layout (points) in; // We only take in a point (the world pos of the grass blade)
layout (triangle_strip, max_vertices = 12) out;

out vec3 mWorldPosition;
out vec3 mNormal;
out vec2 mTextureCoordinates;

uniform vec2 uWidthHeight; // x = width, y = height
uniform int uLODLevel; // 0 == high LOD, 1 == medium LOD, 2 == low LOD

const float HALF_PI = 1.57079632679f;
const vec3 NORMAL = vec3(0.0f, 0.0f, 1.0f);

int fmod(int x, int y); // Returns the remainder of x divided by y with the same sign as x. If y is zero, the result is implementation-defined because of division by zero.

void main()
{
	vec3 worldPos = gl_in[0].gl_Position.xyz;
	int vertexCount = uLODLevel == 0 ? 12 : uLODLevel == 1 ? 8 : 4;
	
	// Generate a random number per-blade to vary width and height
	float random = sin(HALF_PI * fract(worldPos.x) + HALF_PI * fract(worldPos.z)); 
	float width = uWidthHeight.x + (random / 50.0f);
	float height = uWidthHeight.y + (random / 5.0f);
	
	float vertexOffset = 1.0f / ((vertexCount / 2) - 1);
	
	float currentHeight = 0.0f;
	float currentWidth = 0.0f;
	float currentVertex = 0.0f;
	
	for(int i = 0; i < vertexCount; i++)
	{	
		// TODO: Randomly rotate grass (We will need to scale the normal & transformedPosition by the rotation)
		mNormal = NORMAL;
		vec4 transformedPosition;
		if(fmod(i, 2) == 0)
		{
			transformedPosition = vec4(worldPos.x - width, worldPos.y + currentHeight, worldPos.z, 1.0f);
			mTextureCoordinates = vec2(0.0f, currentVertex);
		}
		else
		{
			transformedPosition = vec4(worldPos.x + width, worldPos.y + currentHeight, worldPos.z, 1.0f);
			mTextureCoordinates = vec2(1.0f, currentVertex);
			
			currentVertex += vertexOffset;
			currentHeight = currentVertex * height;
		}
		
		// TODO: Wind
		
		mWorldPosition = transformedPosition.xyz;
		gl_Position = transformedPosition;
		EmitVertex();
	}
	EndPrimitive();
}

int fmod(int x, int y)
{
	float remainder = fract(abs(x / y)) * abs(y);
	return int(x < 0.0f ? -remainder : remainder);
}

//type fragment
#version 420 

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec3 gEffects;

in vec3 mWorldPosition;
in vec3 mNormal;
in vec2 mTextureCoordinates;

uniform sampler2D uAlbedoTexture;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

// Material
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

uniform sampler2D uDiscardTexture;

const float nearPlane = 0.1f;
const float farPlane = 1000.0f;

vec3 LinearizeColor(vec3 color); // Converts to linear color space (sRGB to RGB). In other words, gamma correction https://lettier.github.io/3d-game-shaders-for-beginners/gamma-correction.html
float LinearizeDepth(float depth);
vec3 ComputeTextureNormal();

void main()
{
	if(texture(uDiscardTexture, mTextureCoordinates).r <= 0.0f) // Shouldn't draw pixel here
	{
		discard;
		return;
	}
	
	if(uHasNormalTexture)
	{		
		gNormal.rgb = ComputeTextureNormal(); // Assign normal
	}
	else
	{
		gNormal.rgb = mNormal;
	}
	
	gPosition = vec4(mWorldPosition, LinearizeDepth(gl_FragCoord.z)); // Set position with adjusted depth
	gAlbedo.rgb = texture(uAlbedoTexture, mTextureCoordinates).rgb;
	gEffects.gb = vec2(0.0f, 0.0f);
	
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

vec3 LinearizeColor(vec3 color)
{
	return pow(color.rgb, vec3(2.2f));
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

vec3 ComputeTextureNormal()
{
	vec3 textureNormal = normalize(texture(uNormalTexture, mTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
	
	// Get partial derivatives 
    vec3 dPosX = dFdx(mWorldPosition);
    vec3 dPosY = dFdy(mWorldPosition);
    vec2 dTexX = dFdx(mTextureCoordinates);
    vec2 dTexY = dFdy(mTextureCoordinates);

	// Convert normal to tangent space
    vec3 normal = normalize(mNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * textureNormal);
}