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

uniform sampler2D testTexture;

void main()
{
	vec3 color = texture(testTexture, mTextureCoordinates).rgb;
	oColor = vec4(color, 1.0f);
}