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

uniform sampler2DArray uDepthTexture;
uniform int uLayer;

const float nearPlane = 0.1f;
const float farPlane = 1000.0f;

float LinearizeDepth(float depth);

void main()
{
	float depth = texture(uDepthTexture, vec3(mTextureCoordinates, uLayer)).r;
	oColor = vec4(depth, depth, depth, 1.0f);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));	
}