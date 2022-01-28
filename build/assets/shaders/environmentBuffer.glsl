//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTextureCoordinates;

out vec3 mWorldPosition;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{	
	mWorldPosition = vPosition;
	mat4 rotationView = mat4(mat3(uView));
	vec4 clipPosition = uProjection * rotationView * vec4(mWorldPosition, 1.0f);
	gl_Position  = clipPosition.xyww;
};



//type fragment
#version 420 

layout (location = 0) out vec4 oColor;

in vec3 mWorldPosition;

uniform samplerCube uEnvMap;

void main()
{
	vec3 envColor = textureLod(uEnvMap, mWorldPosition, 0.0).rgb;
	
	// Tonemap and gamma correciton
	envColor = envColor / (envColor + vec3(1.0f));
	envColor = pow(envColor, vec3(1.0f / 2.2f));
	
	oColor = vec4(envColor, 1.0f);
}