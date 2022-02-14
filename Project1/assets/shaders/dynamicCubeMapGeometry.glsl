//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;

uniform bool uIsEnvMap;

out vec3 mWorldPosition;
out vec2 mTextureCoordinates;
out vec3 mNormal;

void main()
{	
	vec4 vertexPos = vec4(vPosition, 1.0f);
	mWorldPosition = (uMatModel * vertexPos).xyz;
	
	mTextureCoordinates = vTextureCoordinates; // Pass out texture coords
	
	// Apply transformation to normal
	mNormal = mat3(uMatModel) * vNormal;
	
	gl_Position = uMatProjection * uMatView * uMatModel * vertexPos;
};



//type fragment
#version 420 

in vec3 mWorldPosition;
in vec2 mTextureCoordinates;
in vec3 mNormal;

out vec4 oColor;

uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;
uniform vec4 uAlbedoRatios;
uniform vec4 uColorOverride;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

const int MAX_LIGHTS = 100;
struct LightInfo
{
	vec3 position;
	vec3 direction;
	vec4 color; // a = intensity
	vec4 param1; // x = light type, y = radius, z = on/off, w = attenuationMode (0 = quadratic, 1 = UE4 style)
};

// Lighitng
uniform int uLightAmount;
uniform LightInfo uLightArray[MAX_LIGHTS];

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

float LinearizeDepth(float depth);
vec3 ComputeTextureNormal();

void main()
{		
	vec3 normal = vec3(0.0f); 
	if(uHasNormalTexture)
	{
		normal.rgb = ComputeTextureNormal(); // Assign normal
	}
	else
	{
		normal.rgb = mNormal;
	}
	
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
	
	for(int i = 0; i < min(uLightAmount, MAX_LIGHTS); i++)
	{
		LightInfo light = uLightArray[i];
			
	}
		
	oColor = vec4(diffuseColor, LinearizeDepth(gl_FragCoord.z)); // Store fragment depth in the alpha channel
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