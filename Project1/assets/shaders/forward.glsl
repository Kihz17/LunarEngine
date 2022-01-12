//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;

out vec3 mNormal;
out vec2 mTextureCoordinates;
out vec3 mWorldPosition;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;
uniform mat4 uMatModelInverseTranspose;

void main()
{	
	mTextureCoordinates = vTextureCoordinates;
	
	// Calculate the normal based on any rotation we've applied.
	// This inverse transpose removes scaling and translation (movement) 
	// 	from the matrix.
	mNormal = vec3(uMatModelInverseTranspose * normalize(vec4(vNormal, 1.0f)));
	mNormal = normalize(mNormal);
	
	mWorldPosition = vec3(uMatModel * vec4(vPosition, 1.0f));
	gl_Position = uMatProjection * uMatView * uMatModel * vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

in vec3 mNormal;
in vec2 mTextureCoordinates;
in vec3 mWorldPosition;

out vec4 oColor;

uniform vec4 uColorOverride; // w = isColorOverride
uniform sampler2D uDiffuse1;
uniform sampler2D uDiffuse2;
uniform sampler2D uDiffuse3;
uniform sampler2D uDiffuse4;
uniform vec4 uDiffuseRatios;
uniform float uAlphaTransparency;

void main()
{
	
	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	if(uColorOverride.w == 1.0f) // Override color
	{
		color.rgb = uColorOverride.rgb;
	}
	else
	{
		color.rgb += texture(uDiffuse1, mTextureCoordinates).rgb * uDiffuseRatios.x;
		color.rgb += texture(uDiffuse2, mTextureCoordinates).rgb * uDiffuseRatios.y;
		color.rgb += texture(uDiffuse3, mTextureCoordinates).rgb * uDiffuseRatios.z;
		color.rgb += texture(uDiffuse4, mTextureCoordinates).rgb * uDiffuseRatios.w;
	}
	
	oColor = vec4(color, uAlphaTransparency);
}