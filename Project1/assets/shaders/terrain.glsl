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

//type tcs
#version 420
layout (vertices = 4) out; // Must match number specified for GL_PATCHES in c++ code

in vec2 mTextureCoordinates[];

out vec2 mTextureCoords[];

uniform mat4 uModel;
uniform mat4 uView;

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

// Used to determine the the amount of tessellation a primitive should have
void main()
{
	// Make sure vertex data get forwarded to the Tessellation Evaluation Shader
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position; // Set gl position going out to what's coming in
    mTextureCoords[gl_InvocationID] = mTextureCoordinates[gl_InvocationID]; // Make sure to pass out our texture coords too

	if(gl_InvocationID == 0) // Invocation 0 is in charge of tessellation for the entire patch
	{
	    const int MIN_TESS_LEVEL = 1;
        const int MAX_TESS_LEVEL = 64;
		const float MIN_DISTANCE = 100;
        const float MAX_DISTANCE = 400;

		// Convert quad vertices to view space
		vec4 pos1 = uView * uModel * gl_in[0].gl_Position;
		vec4 pos2 = uView * uModel * gl_in[1].gl_Position;
		vec4 pos3 = uView * uModel * gl_in[2].gl_Position;
		vec4 pos4 = uView * uModel * gl_in[3].gl_Position;

		// Get distance relative to camera
		float dist1 = clamp((abs(pos1.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0f, 1.0f);
		float dist2 = clamp((abs(pos2.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0f, 1.0f);
		float dist3 = clamp((abs(pos3.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0f, 1.0f);
		float dist4 = clamp((abs(pos4.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0f, 1.0f);

		// Establish tessellation level relative to distance from camera
		float tessLevel1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist3, dist1));
		float tessLevel2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist1, dist2));
		float tessLevel3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist2, dist4));
		float tessLevel4 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist4, dist3));

		// Assign how many segments an edge is tessellated into
		// NOTE: Since we are tessellating quads in this case, ALL outer & inner edges are relevant vs something like a triangle 
		// Examples: Quad: https://gyazo.com/695e9bff178bf6c6e760433606d0d7ae Triangle: https://gyazo.com/d7aafc57fe79673414d72442c2b1daf1

		// Outer level
		gl_TessLevelOuter[0] = tessLevel1;
        gl_TessLevelOuter[1] = tessLevel2;
        gl_TessLevelOuter[2] = tessLevel3;
        gl_TessLevelOuter[3] = tessLevel4;

		// Inner level
		gl_TessLevelInner[0] = max(tessLevel2, tessLevel4);
        gl_TessLevelInner[1] = max(tessLevel1, tessLevel3);

		// Good Example of edge segmentation: https://gyazo.com/e4e47e3ac6633e6f093b43767c5633f4
		// Info can be found here: https://www.khronos.org/opengl/wiki/Tessellation#:~:text=Tessellation%20is%20the%20Vertex%20Processing,and%20a%20fixed%2Dfunction%20stage.
	}
}

//type tes
#version 420

layout (quads, fractional_odd_spacing, ccw) in;

in vec2 mTextureCoords[];

out vec2 mTexCoords;
out vec3 mWorldPosition;
out vec3 mNormal;

uniform vec2 uSeed;
uniform vec4 uTerrainParams; // x = amplitude, y = roughness, z = persistence, w = freq
uniform int uOctaves;

uniform mat4 uModel;
uniform mat4 uProjection;
uniform mat4 uView;
uniform vec2 uCameraPosition;

float Rand(vec2 co); // Generates a pseudo random number https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float SmoothNoise(int x, int y); // Generates a pseudo random "smoothed" noise value
float InterpolateCos(float a, float b, float blend); // Cosine interpolation for 2 values
float InterpolateNoise(vec2 v);
float GetHeight(vec2 v);

const float PI = 3.14159265359f;

void main()
{
	// Get texture coords from TCS
	vec2 uv1 = mTextureCoords[0];
	vec2 uv2 = mTextureCoords[1];
	vec2 uv3 = mTextureCoords[2];
	vec2 uv4 = mTextureCoords[3];

	// Bilinearly interpolate texture coordinates across patch
	vec2 uvDelta1 = (uv2 - uv1) * gl_TessCoord.x + uv1;
	vec2 uvDelta2 = (uv4 - uv3) * gl_TessCoord.x + uv3;
	mTexCoords = (uvDelta2 - uvDelta1) * gl_TessCoord.y + uvDelta1;

	// Get vertex positions from TCS
	vec4 v1 = gl_in[0].gl_Position;
    vec4 v2 = gl_in[1].gl_Position;
    vec4 v3 = gl_in[2].gl_Position;
    vec4 v4 = gl_in[3].gl_Position;

	// Compute the normal for this patch
	vec4 a = v2 - v1;
	vec4 b = v3 - v1;
	vec4 normal = normalize(vec4(cross(b.xyz, a.xyz), 0.0f));

	// Bilinearly interpolate vertices across patch
	vec4 vDelta1 = (v2 - v1) * gl_TessCoord.x + v1;
	vec4 vDelta2 = (v4 - v3) * gl_TessCoord.x + v3;
	vec4 v = (vDelta2 - vDelta1) * gl_TessCoord.y + vDelta1;

	vec2 vertexPos = vec2(v.x, v.z);
	float heightResult = GetHeight(vertexPos);	// Get height value

	v += normal * heightResult; // Push vertex position in direction of normal scaled by height

	float heightLeft = GetHeight(vertexPos - vec2(1.0f, 0.0f));
	float heightRight = GetHeight(vertexPos + vec2(1.0f, 0.0f));
	float heightUp = GetHeight(vertexPos + vec2(0.0f, 1.0f));
	float heightDown = GetHeight(vertexPos - vec2(0.0f, 1.0f));
	mNormal = normalize(vec3(heightLeft - heightRight, 1.0f, heightDown - heightUp));

	vec4 worldPos = uModel * v;
	mWorldPosition = worldPos.xyz; 

	gl_Position = uProjection * uView * worldPos;
}

float Rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233) + uSeed)) * 43758.5453);
}

float SmoothNoise(int iX, int iY)
{
	float cornerContrib = (Rand(vec2(iX - 1, iY - 1)) +  Rand(vec2(iX + 1, iY - 1)) +  Rand(vec2(iX + 1, iY + 1)) +  Rand(vec2(iX - 1, iY + 1))) / 16.0f;
	float sideContrib = (Rand(vec2(iX - 1, iY)) +  Rand(vec2(iX, iY - 1)) +  Rand(vec2(iX + 1, iY)) +  Rand(vec2(iX, iY + 1))) / 8.0f;
	float centerContrib = Rand(vec2(iX, iY)) / 4.0f;

	return cornerContrib + sideContrib + centerContrib;
}

float InterpolateCos(float a, float b, float blend)
{
	float theta = blend * PI;
	float blendFactor = (1.0f - cos(theta)) * 0.5f;
	return a * (1.0f - blendFactor) + b * blendFactor;
}

float InterpolateNoise(vec2 v)
{
	int iX = int(floor(v.x));
	int iY = int(floor(v.y));
	float fractX = fract(v.x);
	float fractY = fract(v.y);

	// Generate smoothed noise values
	float a = SmoothNoise(iX, iY);
	float b = SmoothNoise(iX + 1, iY);
	float c = SmoothNoise(iX, iY + 1);
	float d = SmoothNoise(iX + 1, iY + 1);

	// Interpolate between smoothed noise
	float i1 = InterpolateCos(a, b, fractX);
	float i2 = InterpolateCos(c, d, fractX);

	return InterpolateCos(i1, i2, fractY); // Interpolate between interpolated values
}

float GetHeight(vec2 v)
{
	float heightResult = 0.0f;
	float amplitude = uTerrainParams.x;
	float freq = uTerrainParams.w;
	vec2 pos = uCameraPosition + v;
	for(int i = 0; i < uOctaves; i++)
	{
		freq *= 2.0f;
		amplitude *= uTerrainParams.z;

		vec2 p = freq * pos;
		heightResult += InterpolateNoise(p) * amplitude;
	}

	return heightResult;
}





//type fragment
#version 420 

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gEffects;

in vec2 mTexCoords;
in vec3 mWorldPosition;
in vec3 mNormal;

uniform sampler2D uTerrainTexture;
uniform float uTextureCoordScale;

float LinearizeDepth(float depth);

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

void main()
{	
	gPosition = vec4(mWorldPosition, LinearizeDepth(gl_FragCoord.z));
	gAlbedo = vec4(texture(uTerrainTexture, mTexCoords * uTextureCoordScale).rgb, 0.9f);
	gNormal = vec4(mNormal, 0.001f);
	gEffects = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}