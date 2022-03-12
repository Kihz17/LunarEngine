//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoordinates;

out vec2 mTextureCoordinates;
out vec3 mViewVector;
out vec3 mWorldPosition;
out vec3 mOrigin;

uniform mat4 uMatView;
uniform mat4 uMatProjection;

const float FAR_PLANE = 1000.0f;
const float NEAR_PLANE = 0.1f;

void main()
{	
	mTextureCoordinates = vTextureCoordinates; // Pass out texture coords

	// Compute ray origin and direction (this will allow us to have a directional vector pointing from cameraPos -> fragment so that we can ray trace)
	// Taken from https://stackoverflow.com/questions/2354821/raycasting-how-to-properly-apply-a-projection-matrix
	mat4 inverseProjView = inverse(uMatProjection * uMatView);
	mOrigin = (inverseProjView * vec4(vPosition.x, vPosition.y, -1.0f, 1.0f) * NEAR_PLANE).xyz;
	mViewVector = (inverseProjView * vec4(vPosition.xy * (FAR_PLANE - NEAR_PLANE), FAR_PLANE + NEAR_PLANE, FAR_PLANE - NEAR_PLANE)).xyz;
	
	gl_Position = vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

in vec2 mTextureCoordinates;
in vec3 mViewVector;
in vec3 mOrigin;

out vec4 oColor;

uniform vec3 uBoxCenter;
uniform vec3 uBoxExtents;

uniform sampler2D uPositionBuffer;

uniform vec3 uLightDirection;
uniform vec3 uLightColor;

uniform sampler3D uShapeNoise;
uniform sampler3D uDetailNoise;
uniform sampler2D uOffsetTexture;

uniform vec2 uScreenDimesnions;

// RAY MACHING SETTINGS
uniform int uLightStepCount;
uniform float uRayOffsetStrength;

// CLOUD SHAPE SETTINGS
uniform float uCloudScale;
uniform float uDensityMultiplier;
uniform float uDensityOffset;
uniform vec3 uShapeOffset;
uniform vec4 uShapeNoiseWeights;
uniform vec4 uPhaseParams;

// CLOUD LIGHTING SETTINGS
uniform float uLightAbsorptionThroughCloud;
uniform float uLightAbsorptionTowardsSun;
uniform float uDarknessThreshold;

// CLOUD DETAIL SETTINGS
uniform float uDetailNoiseScale;
uniform float uDetailNoiseWeight;
uniform vec3 uDetailNoiseWeights;
uniform vec3 uDetailOffset;

// TIMELAPSE SETTINGS
uniform float uTimeScale;
uniform float uTime;
uniform float uSpeed;
uniform float uDetailSpeed;

vec2 DistanceToBox(vec3 boundsMin, vec3 boundsMax, vec3 origin, vec3 direction); // Returns the distance to the box given a ray
vec2 SquareTextureCoords(vec2 texCoords);
float SampleDensityAtPoint(vec3 pos, vec3 boundsMin, vec3 boundsMax); // Gets cloud density at a given point
float Remap(float v, float minOld, float maxOld, float minNew, float maxNew);
float HenyeyGreenstein(float a, float b); // Phase function by HenyeyGreenstein. Gives us the angular distribution of light intensity at a given wavelength
float LightMarch(vec3 pos, vec3 boundsMin, vec3 boundsMax); // Computes the portion of light that will reach the given point from the lightsource

const float FAR_PLANE = 1000.0f;
const float NEAR_PLANE = 0.1f;

void main()
{
	vec4 sampledPositionBuffer = texture(uPositionBuffer, mTextureCoordinates);
	
	vec3 worldPos = sampledPositionBuffer.xyz;
	float depth = sampledPositionBuffer.w;
	if (depth == 1.0f) // No depth here, consider the depth to be equal to our far plane
	{
		depth = FAR_PLANE - NEAR_PLANE;
	}

	// Create ray
	vec3 rayOrigin = mOrigin;
	vec3 rayDirection = normalize(mViewVector);
	
	const vec3 boundsMin = uBoxCenter - uBoxExtents;
	const vec3 boundsMax = uBoxCenter + uBoxExtents;

	// Check if ray collides with out "cloud box"
	vec2 rayToBoxInfo = DistanceToBox(boundsMin, boundsMax, rayOrigin, 1.0f / rayDirection);
	float distToBox = rayToBoxInfo.x;
	float distInsideBox = rayToBoxInfo.y;
	
	bool rayHitBox = distInsideBox > 0.0f && distToBox < depth;
	if (!rayHitBox) // This pixel's ray didn't even collide with out cloud box, no need to do anything else
	{
		oColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	vec3 pointOfIntersection = rayOrigin + rayDirection * distToBox; // Represents the point where we first intersect with our cloud box

	// Phase function (makes clouds brighter around the source of light)
	vec3 lightWorldPos = -uLightDirection * (FAR_PLANE - NEAR_PLANE); // Push our light position to the edge of the view frustum
	float cosAngle = dot(rayDirection, lightWorldPos);
	const float blend = 0.5f;
	float hgBlend = HenyeyGreenstein(cosAngle, uPhaseParams.x) * (1.0f - blend) + HenyeyGreenstein(cosAngle, -uPhaseParams.y) * blend;
	float phaseValue = uPhaseParams.z + hgBlend * uPhaseParams.w;

	// Begin ray marching
	float distanceTravelled = textureLod(uOffsetTexture, SquareTextureCoords(mTextureCoordinates * 3.0f), 0).r * uRayOffsetStrength; // Generate a random starting offset
	float distanceLimit = min(depth - distToBox, distInsideBox); // Only allow travelling WITHIN the cloud box

	const float stepSize = 11;

	float transmittance = 1.0f;
	vec3 lightEnergy = vec3(0.0f, 0.0f, 0.0f);
	while (distanceTravelled < distanceLimit)
	{
		rayOrigin = pointOfIntersection + rayDirection * distanceTravelled; // Move our ray origin along
		float density = SampleDensityAtPoint(rayOrigin, boundsMin, boundsMax); // Sample density from this point within cloud

		if (density > 0) // Have a bit of cloud here
		{
			oColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
			return;
			float lightTransmittance = LightMarch(rayOrigin, boundsMin, boundsMax); // March along the ray to get the amount of light that seeps through from us -> the light source
			lightEnergy += density * stepSize * transmittance * lightTransmittance * phaseValue;
			transmittance *= exp(-density * stepSize * uLightAbsorptionThroughCloud);

			if (transmittance < 0.01f) // Leave loop early if transmittence is low since continuing to sample won't affect the end result much.
			{
				break;
			}
		}
		distanceTravelled += stepSize; // Make sure we progress our march forward
	}

	const float baseScale = 1.0f / 1000.0;
	const float offsetSpeed = 1.0f / 100.0;

	vec3 cloudColor = transmittance + (lightEnergy * uLightColor); // Our result!
	oColor = vec4(cloudColor, 1.0f);
}

vec2 DistanceToBox(vec3 boundsMin, vec3 boundsMax, vec3 origin, vec3 invRay)
{
	vec3 t0 = (boundsMin - origin) * invRay;
	vec3 t1 = (boundsMax - origin) * invRay;
	vec3 tMin = min(t0, t1);
	vec3 tMax = max(t0, t1);
	
	float distA = max(max(tMin.x, tMin.y), tMin.z);
	float distB = min(tMax.x, min(tMax.y, tMax.z));
	
	float distToBox = max(0.0f, distA);
	float distToInside = max(0.0f, distB - distToBox);
	return vec2(distToBox, distToInside);
}

vec2 SquareTextureCoords(vec2 texCoords)
{
	const float scale = 1000.0f;
	float x = texCoords.x * uScreenDimesnions.x;
	float y = texCoords.y * uScreenDimesnions.y;
	return vec2(x / scale, y / scale);
}

float SampleDensityAtPoint(vec3 pos, vec3 boundsMin, vec3 boundsMax)
{
	const float baseScale = 1.0f / 1000.0;
	const float offsetSpeed = 1.0f / 100.0;

	// Compute texture coordinates
	float time = uTime * uTimeScale;
	vec3 uvw = (uBoxExtents * 0.5f + pos) * baseScale * uCloudScale;
	vec3 shapeTexCoords = uvw + uShapeOffset * offsetSpeed + vec3(uTime, uTime * 0.1f, uTime * 0.2f) * uSpeed;

	// Compute fallof at the edges of the cloud box
	const float containerEdgeFade = 50.0f;
	float distFromEdgeX = min(containerEdgeFade, min(pos.x - boundsMin.x, boundsMax.x - pos.x));
	float distFromEdgeZ = min(containerEdgeFade, min(pos.z - boundsMin.z, boundsMax.z - pos.z));
	float edgeWeight = min(distFromEdgeZ, distFromEdgeX) / containerEdgeFade;

	const float gMin = 0.2f;
	const float gMax = 0.7f;
	float heightPercent = (pos.y - boundsMin.y) / uBoxExtents.y;
	float heightGradient = clamp(Remap(heightPercent, 0.0, gMin, 0.0f, 1.0f), 0.0f, 1.0f) * clamp(Remap(heightPercent, 1, gMax, 0.0f, 1.0f), 0.0f, 1.0f);
	heightGradient *= edgeWeight;

	// Compute base shape density
	vec4 sampledNoise = textureLod(uShapeNoise, shapeTexCoords, 0);
	vec4 normalizedNoise = uShapeNoiseWeights / dot(uShapeNoiseWeights, vec4(1.0f, 1.0f, 1.0f, 1.0f));
	float shape = dot(sampledNoise, normalizedNoise) * heightGradient;
	float baseDensity = shape + uDensityOffset * 0.1f;

	if (baseDensity > 0.0f) // Only sample if we have some density here
	{
		// Sample from detail noise texture
		vec3 detailTexCoords = uvw * uDetailNoiseScale * uDetailOffset * offsetSpeed + vec3(uTime * 0.4f, -uTime, uTime * 0.1f) * uDetailSpeed;
		vec4 sampledDetailNoise = textureLod(uDetailNoise, detailTexCoords, 0);
		vec3 normalizedDetailNoise = uDetailNoiseWeights / dot(uDetailNoiseWeights, vec3(1.0f, 1.0f, 1.0f));
		float detail = dot(sampledDetailNoise.rgb, normalizedDetailNoise);

		// Subtract detail noise from the "shape" (weighted by inverse density so that edges are more eroded than the center)
		float shapeContrib = 1.0f - shape;
		float shapeContrib3 = shapeContrib * shapeContrib * shapeContrib;
		float cloudDensity = baseDensity - (1.0f - detail) * shapeContrib3 * uDetailNoiseWeight;

		return cloudDensity * uDensityMultiplier * 0.1f;
	}

	return 0.0f; // No density
}

float Remap(float v, float minOld, float maxOld, float minNew, float maxNew)
{
	return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}

float HenyeyGreenstein(float a, float g)
{
	float g2 = g * g;
	return (1.0f - g2) / (4.0f * 3.1415f * pow(1.0f + g2 - 2.0f * g * (a), 1.5f));
}

float LightMarch(vec3 pos, vec3 boundsMin, vec3 boundsMax)
{
	vec3 dirToLight = normalize(-uLightDirection);
	float distInsideBox = DistanceToBox(boundsMin, boundsMax, pos, 1.0f / dirToLight).y;

	float stepSize = distInsideBox / uLightStepCount;

	// Step through the cloud and accumulate denisty at various points
	float density = 0.0f;
	for (int i = 0; i < uLightStepCount; i++)
	{
		pos += dirToLight * stepSize;
		density += max(0.0f, SampleDensityAtPoint(pos, boundsMin, boundsMax) * stepSize);
	}

	float transmittance = exp(-density * uLightAbsorptionTowardsSun);
	return uDarknessThreshold + transmittance * (1.0f - uDarknessThreshold);
}