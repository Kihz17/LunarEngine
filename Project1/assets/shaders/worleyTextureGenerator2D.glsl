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

uniform float uResolution;
uniform int uNumWorleyPoints;
uniform vec2 uPointData[256];

void main()
{
	vec2 fragPos = gl_FragCoord.xy / uResolution; // Convert frag pos to range 0 - 1

	// Find closest point's distance
	float closestDistance = -1.0f;
	for (int i = 0; i < uNumWorleyPoints; i++)
	{
		vec2 p = uPointData[i] / uResolution; // Convert point to range 0 - 1

		float dist = distance(fragPos, p);
		if (closestDistance == -1.0f || dist < closestDistance)
		{
			closestDistance = dist;
		}
	}

	if (closestDistance != -1.0f)
	{
		float c = closestDistance;
		oColor = vec4(c, c, c, 1.0f);
	}
	else // Failsafe
	{
		oColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}