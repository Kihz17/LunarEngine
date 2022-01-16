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

out vec3 mNormal;
out vec2 mTextureCoordinates;

uniform vec2 uWidthHeight; // x = width, y = height
uniform int uLODLevel; // 0 == high LOD, 1 == medium LOD, 2 == low LOD

const float HALF_PI = 1.57079632679f;
const vec3 NORMAL = vec3(0.0f, 0.0f, 1.0f);

int fmod(int x, int y); // Returns the remainder of x divided by y with the same sign as x. If y is zero, the result is implementation-defined because of division by zero.

void main()
{
	vec3 worldPos = gl_in[0].gl_Position;
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
		mNormal = NORMAL;
		if(fmod(i, 2) == 0)
		{
			gl_Position = vec4(worldPos.x - width, worldPos.y + currentHeight, worldPos.z, 1.0f);
			mTextureCoordinates = vec2(0.0f, currentVertex);
		}
		else
		{
			gl_Position = vec4(worldPos.x + width, worldPos.y + currentHeight, worldPos.z, 1.0f);
			mTextureCoordinates = vec2(1.0f, currentVertex);
			
			currentVertex += vertexOffset;
			currentHeight = currentVertex * height;
		}
		
		// TODO: Wind
		
		EmitVertex();
	}
}

int fmod(int x, int y)
{
	float remainder = fract(abs(x / y)) * abs(y);
	return (int) (x < 0.0f ? -remainder : remainder);
}

//type fragment
#version 420 

in vec3 mNormal;
in vec2 mTextureCoordinates;

out vec4 oColor;

uniform sampler2D uAlbedoTexture;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

// Material
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

vec3 ComputeTextureNormal(vec3 viewNormal, vec3 textureNormal);

void main()
{
	vec3 normal;
	if(uHasNormalTexture)
	{
		normal = normalize(texture(uNormalTexture, mTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
		
		mat3 matNormal = transpose(inverse(mat3(uMatView * uMatModel)));
		
		
		normal = ComputeTextureNormal(mNormal, normal); // Assign normal
	}
	else
	{
		normal = mNormal;
	}
	
	vec3 color = texture(testTexture, mTextureCoordinates).rgb;
	oColor = vec4(color, 1.0f);
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