//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoordinates;

out vec2 mTextureCoordinates;

void main()
{	
	mTextureCoordinates = vTextureCoordinates; // Pass out texture coords
	gl_Position = vec4(vPosition, 1.0f);
};


//type fragment
#version 420 

in vec2 mTextureCoordinates;

out vec4 oColor;

uniform sampler2D uCloudsTexture;
uniform sampler2D uEmissionTexture;

uniform bool uEnableGodRays;
uniform vec3 uLightPosition;
uniform float uLightDotCameraDir;

uniform vec2 uResolution;
uniform vec2 uCloudResolution;

vec4 GaussianBlur(sampler2D t, vec2 texCoords);

const float FAR_PLANE = 1000.0f;
const float NEAR_PLANE = 0.1f;

void main()
{
	oColor = GaussianBlur(uCloudsTexture, mTextureCoordinates);

	// God rays
	if(uLightDotCameraDir > 0.0f && uEnableGodRays)
	{

	}
}

vec4 GaussianBlur(sampler2D t, vec2 texCoords)
{
	vec2 offset =  1.0f / uCloudResolution;
	const vec2 offsets[9] = vec2[]
	(
	    vec2(-offset.x, offset.y), // top-left
        vec2(0.0f, offset.y), // top-center
        vec2(offset.x, offset.y), // top-right
        vec2(-offset.x, 0.0f),   // center-left
        vec2(0.0f, 0.0f),   // center-center
        vec2(offset.x, 0.0f),   // center-right
        vec2(-offset.x, -offset.y), // bottom-left
        vec2(0.0f, -offset.y), // bottom-center
        vec2(offset.x, -offset.y)  // bottom-right   
	);

	const float kernel[9] = float[]
	(
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16  
	);

	// Sample neighbouring 9 pixels
	vec4 sampledValues[9];
	for(int i = 0; i < 9; i++)
	{
		sampledValues[i] = texture(t, texCoords + offsets[i]);
	}

	// Blur between all sampled values
	vec4 color = vec4(0.0f);
	for(int i = 0; i < 9; i++)
	{
		color += sampledValues[i] * kernel[i];
	}

	return color;
}