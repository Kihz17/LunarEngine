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

uniform sampler2D uFaceBuffer;
uniform samplerCube uEnvMap;
uniform int uCubeFace; // 0 = PosX, 1 = NegX, 2 = PosY, 3 = NegY, 4 = PosZ, 5 = NegZ

void main()
{	
	vec4 sceneGeometrySample = texture(uFaceBuffer, mTextureCoordinates);
	
	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	if(sceneGeometrySample.a == 1.0f) // No depth, sample from environment map
	{
		vec2 mapCoords = 2.0f * mTextureCoordinates - 1.0f; // Convert texture coordinates to range from [-1.0, 1.0f]
		
		if(uCubeFace == 0) // PosX
		{
			color = texture(uEnvMap, vec3(1.0, mapCoords.y, mapCoords.x)).rgb; 
		}
		else if(uCubeFace == 1) // NegX
		{
			color = texture(uEnvMap, vec3(-1.0, mapCoords.y, mapCoords.x)).rgb; 
		}
		else if(uCubeFace == 2) // PosY
		{	
			color = texture(uEnvMap, vec3(mapCoords.x, 1.0f, mapCoords.y)).rgb; 
		}
		else if(uCubeFace == 3) // NegY
		{
			color = texture(uEnvMap, vec3(mapCoords.x, -1.0f, mapCoords.y)).rgb; 
		}
		else if(uCubeFace == 4) // PosZ
		{
			color = texture(uEnvMap, vec3(mapCoords.xy, 1.0f)).rgb; 
		}
		else if(uCubeFace == 5) // NegZ
		{
			color = texture(uEnvMap, vec3(mapCoords.xy, -1.0f)).rgb; 
		}
	}
	else
	{
		color = sceneGeometrySample.rgb;
	}
	
	oColor = vec4(color, 1.0f);
}