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

uniform int uCurrentSlice;
uniform float uResolution;
uniform int uNumWorleyPointsR;
uniform int uNumWorleyPointsG;
uniform int uNumWorleyPointsB;
uniform vec3 uPointDataR[256];
uniform vec3 uPointDataG[256];
uniform vec3 uPointDataB[256];

void main()
{
	vec3 fragPos = vec3(gl_FragCoord.xy, uCurrentSlice) / uResolution; // Convert frag pos to range 0 - 1

	// Find closest point's distance
	vec3 closestDist = vec3(100000.0f, 100000.0f, 100000.0f);

	// Get closest point for red channel
	for (int i = 0; i < uNumWorleyPointsR; i++)
	{
		vec3 p = uPointDataR[i] / uResolution; // Convert point to range 0 - 1

		float dist = distance(fragPos, p);
		if (dist < closestDist.r)
		{
			closestDist.r = dist;
		}
	}

	// Get closest point for green channel
	//for (int i = 0; i < uNumWorleyPointsG; i++)
	//{
	//	vec3 p = uPointDataG[i] / uResolution; // Convert point to range 0 - 1

	//	float dist = distance(fragPos, p);
	//	if (closestDist.g == -1.0f || dist < closestDist.g)
	//	{
	//		closestDist.g = dist;
	//	}
	//}

	//// Get closest point for blue channel
	//for (int i = 0; i < uNumWorleyPointsB; i++)
	//{
	//	vec3 p = uPointDataB[i] / uResolution; // Convert point to range 0 - 1

	//	float dist = distance(fragPos, p);
	//	if (closestDist.b == -1.0f || dist < closestDist.b)
	//	{
	//		closestDist.b = dist;
	//	}
	//}

	oColor = vec4(closestDist.r, closestDist.r, closestDist.r, 1.0f);
}