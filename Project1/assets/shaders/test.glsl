//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoordinates;

out vec2 oTextureCoordinates;

uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

void main()
{	
	oTextureCoordinates = vTextureCoordinates; // Pass out texture coords
	
	gl_Position = vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

in vec2 iTextureCoordinates;

out vec4 oColor;

uniform sampler2D testTexture;

void main()
{
	//vec3 color = texture(testTexture, iTextureCoordinates).rgb;
	oColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}