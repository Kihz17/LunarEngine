#version 430 

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D uColorBuffer;
layout(rgba32f, binding = 1) uniform image2D uBloomBuffer;

// Cloud textures
uniform sampler3D uCloudTexture;
uniform sampler3D uWorleyTexture;
uniform sampler2D uWeatherTexture;
uniform sampler2D uSkyTexture;

uniform vec2 uResolution;
uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform vec3 uCameraPosition;

uniform vec3 uLightDirection;
uniform vec3 uLightColor;

// Cloud params
uniform float uCrispiness = 0.4f;
uniform float uDensityFactor = 0.02f;
uniform float uCloudCoverageMult = 0.4f;
uniform float uDetail;
uniform float uAbsorptionToLight = 0.0035f;
uniform float uCloudDarknessMult = 1.5f;
uniform vec3 uCloudColorTop = vec3(169.0f, 149.0f, 149.0f) * (1.5f / 255.0f);
uniform vec3 uCloudColorBottom = vec3(65.0f, 70.0f, 80.0f)*(1.5f / 255.0f);

uniform float uTime;
uniform float uCloudSpeed;
uniform vec3 uWindDirection = normalize(vec3(0.5f, 0.0f, 0.1f));

uniform float uEarthRadius = 600000.0f;
uniform float uSphereInnerRadius = 5000.0;
uniform float uSphereOuterRadius = 17000.0;

// Offsets for cone sampling
uniform vec3 uNoiseKernel[6] = vec3[]
(
	vec3( 0.38051305f,  0.92453449f, -0.02111345f),
	vec3(-0.50625799f, -0.03590792f, -0.86163418f),
	vec3(-0.32509218f, -0.94557439f,  0.01428793f),
	vec3( 0.09026238f, -0.27376545f,  0.95755165f),
	vec3( 0.28128598f,  0.42443639f, -0.86065785f),
	vec3(-0.16852403f,  0.14748697f,  0.97460106f)
);

#define BAYER_FACTOR 1.0f / 16.0f
uniform float uBayerFilter[16] = float[]
(
	0.0f * BAYER_FACTOR, 8.0f * BAYER_FACTOR, 2.0f * BAYER_FACTOR, 10.0f * BAYER_FACTOR,
	12.0f * BAYER_FACTOR, 4.0f * BAYER_FACTOR, 14.0f * BAYER_FACTOR, 6.0f * BAYER_FACTOR,
	3.0f * BAYER_FACTOR, 11.0f * BAYER_FACTOR, 1.0f * BAYER_FACTOR, 9.0f * BAYER_FACTOR,
	15.0f * BAYER_FACTOR, 7.0f * BAYER_FACTOR, 13.0f * BAYER_FACTOR, 5.0f * BAYER_FACTOR
);

const float CLOUD_TOP_OFFSET = 750.0f;

// Earth atmosphere ranges
#define EARTH_RADIUS uEarthRadius
#define SPHERE_INNER_RADIUS (EARTH_RADIUS + uSphereInnerRadius)
#define SPHERE_OUTER_RADIUS (SPHERE_INNER_RADIUS + uSphereOuterRadius)
#define SPHERE_DELTA float(SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS)

// Cloud types height density gradients
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)

#define CLOUDS_MIN_TRANSMITTANCE 1e-1

vec3 worldSphereCenter = vec3(0.0f, -EARTH_RADIUS, 0.0f);

vec3 ComputeClipSpaceCoord(uvec2 fragCoord);
vec3 RayIntersectSkySphere(vec3 rayDir, float radius); // Get the position where a given ray intersects with the sky sphere
bool RaySphereIntersect(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float radius, out vec3 pos); // Returns the position where a given ray intersects with a sphere
float ComputeFog(vec3 startPos, float factor); // Computes fog density at a given starting point
float GetHeightFraction(vec3 pos);
float Remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax);
float RaymarchToLight(vec3 origin, float stepSize, vec3 lightDir, float originalDensity, float lightDotEye); // Raymarch towards light
float SampleDensityAtPoint(vec3 point, bool expensive, float lod); // Sample cloud density at a given point
float HenyeyGreenstein(float a, float g); // Phase function by HenyeyGreenstein. Gives us the angular distribution of light intensity at a given wavelength

void main()
{
	ivec2 fragCoord = ivec2(gl_GlobalInvocationID.xy);
	vec4 rayClip = vec4(ComputeClipSpaceCoord(fragCoord), 1.0f);
	vec4 rayView = uInvProj * rayClip;
	rayView = vec4(rayView.xy, -1.0f, 0.0f);
	vec3 rayDir = normalize((uInvView * rayView).xyz);

	vec3 cubeMapEnd = RayIntersectSkySphere(rayDir, 0.5f); // The position where our ray intersects w/ skybox

	vec4 background = texture(uSkyTexture, fragCoord / uResolution);

	worldSphereCenter.xz = uCameraPosition.xz; // Make sure the "world center" follows the camera around

	vec3 startPos; // Represents the start position for ray marching
	vec3 endPos; // Represents the end position for ray marching
	vec3 fogRay;

	if(uCameraPosition.y < SPHERE_INNER_RADIUS - EARTH_RADIUS) // We are within the earth's "inner radius"
	{
		RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_INNER_RADIUS, startPos);
		RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_OUTER_RADIUS, endPos);
		fogRay = startPos;
	}
	else if(uCameraPosition.y > SPHERE_INNER_RADIUS - EARTH_RADIUS && uCameraPosition.y < SPHERE_OUTER_RADIUS - EARTH_RADIUS) // Outside the "inner" bounds but within outer bounds
	{
		startPos = uCameraPosition;
		RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_OUTER_RADIUS, endPos);
		if(!RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_INNER_RADIUS, fogRay))
		{
			fogRay = startPos;
		}
	}
	else
	{
		RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_OUTER_RADIUS, startPos);
		RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_INNER_RADIUS, endPos);
		RaySphereIntersect(uCameraPosition, rayDir, worldSphereCenter, SPHERE_OUTER_RADIUS, fogRay);
	}

	float fogAmount = ComputeFog(fogRay, 0.00008f); // Compute fog

	vec4 fragValue = background; // Set the color of this fragment to be the background

	// Compute sun bloom
	float bloomSun = clamp(dot(uLightDirection, rayDir), 0.0f, 1.0f);
	vec3 bloomSunColor = 0.8f * uLightColor * pow(bloomSun, 128);
	vec4 bloomValue = vec4(bloomSunColor * 1.3f, 1.0f);

	if(fogAmount > 0.965f) // Lots of fog, no point in going any further since fog is obstructing eveything
	{
		bloomValue = background;
		imageStore(uColorBuffer, fragCoord, fragValue);
		imageStore(uBloomBuffer, fragCoord, bloomValue);
		return;
	}

	const int numSteps = 64;

	// Get direction to march in
	vec3 dir = endPos - startPos;
	float len = length(dir);
	float dist = len / numSteps;
	dir = (dir / len) * dist;

	// Start us off in a pseudo random spot
	int a = int(fragCoord.x) % 4;
	int b = int(fragCoord.y) % 4;
	startPos += dir * uBayerFilter[a * 4 + b];

	float lightDotEye = dot(normalize(uLightDirection), normalize(dir));

	float T = 1.0f;
	float sigmaDist = -dist * uDensityFactor;

	bool expensive = true;
	bool entered = false;

	// Start ray marching towards cloud
	vec4 rayMarchValue = vec4(0.0f);
	vec3 marchPos = startPos;
	for(int i = 0; i < numSteps; i++)
	{
		float densitySample = SampleDensityAtPoint(marchPos, true, i / 16); // Sample density at point
		if(densitySample > 0.0f) // Some density here
		{
			float height = GetHeightFraction(marchPos);
			float lightDensity = RaymarchToLight(marchPos, dist * 0.1f, uLightDirection, densitySample, lightDotEye);
			float scattering = mix(HenyeyGreenstein(lightDotEye, -0.08f), HenyeyGreenstein(lightDotEye, 0.08f), clamp(lightDotEye * 0.5f + 0.5f, 0.0f, 1.0f));
			scattering = max(scattering, 1.0f);

			const vec3 sunColor = uLightColor * vec3(1.1f, 1.1f, 0.95f);
			vec3 S = 0.6f * (mix(mix(uCloudColorBottom * 1.8f, background.rgb, 0.2f), scattering * sunColor, lightDensity)) * densitySample;
			float transmittence = exp(densitySample * sigmaDist);
			vec3 Sint = (S - S * transmittence) * (1.0f / densitySample);
			rayMarchValue.rgb += T * Sint;
			T *= transmittence;
		}

		if(T <= CLOUDS_MIN_TRANSMITTANCE) break;

		marchPos += dir; // Move forward in march
	}

	rayMarchValue.a = 1.0f - T;
	rayMarchValue.rgb *= uCloudDarknessMult;

	// Fade clouds in the distance
	//rayMarchValue.rgb = mix(rayMarchValue.rgb, background.rgb * rayMarchValue.a, clamp(fogAmount, 0.0f, 1.0f));

	// Sun glare
	float sun = clamp(dot(uLightDirection, normalize(endPos - startPos)), 0.0f, 1.0f);
	vec3 sunColor = 0.5f * uLightColor * pow(sun, 256.0f);
	rayMarchValue.rgb += sunColor * rayMarchValue.a;

	// Blend clouds and background 
	background.rgb = background.rgb * (1.0f - rayMarchValue.a) + rayMarchValue.rgb;
	background.a = 1.0f;
	fragValue = background; // Set final "background" color

	float cloudAlphaness = rayMarchValue.a > 0.2f ? rayMarchValue.a : 0.0f;
	if(cloudAlphaness > 0.1f) // Apply fog to bloom buffer
	{
		float fogAmount = ComputeFog(startPos, 0.0003f);
		vec3 cloud = mix(vec3(0.0f), bloomValue.rgb, clamp(fogAmount, 0.0f, 1.0f));
		bloomValue.rgb = bloomValue.rgb * (1.0f - cloudAlphaness) + cloud.rgb;
	}
	fragValue.a = cloudAlphaness;

	imageStore(uColorBuffer, fragCoord, fragValue);
	imageStore(uBloomBuffer, fragCoord, bloomValue);
}

vec3 ComputeClipSpaceCoord(uvec2 fragCoord)
{
	vec2 rayNDC = 2.0f * vec2(fragCoord.xy) / uResolution.xy - 1.0f;
	return vec3(rayNDC, 1.0f);
}

vec3 RayIntersectSkySphere(vec3 rayDir, float radius)
{
	float r2 = radius * radius;

	vec3 sphereCenter = vec3(0.0f);
	vec3 L = -sphereCenter;

	float a = dot(rayDir, rayDir);
	float b = 2.0f * dot(rayDir, L);
	float c = dot(L, L) - r2;

	float discr = b * b - 4.0f * a * c;
	float t = max(0.0f, (-b + sqrt(discr)) / 2.0f);

	return rayDir * t;
}

bool RaySphereIntersect(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float radius, out vec3 pos)
{
	float r2 = radius * radius;

	vec3 L = rayOrigin - worldSphereCenter;
	float a = dot(rayDir, rayDir);
	float b = 2.0f * dot(rayDir, L);
	float c = dot(L, L) - r2;

	float discr = b * b - 4.0f * a * c;
	if(discr < 0.0f) return false;

	float t = max(0.0f, (-b + sqrt(discr)) / 2.0f);
	if(t == 0.0f) return false; // No intersection point

	pos = rayOrigin + rayDir * t;
	return true;
}

float ComputeFog(vec3 startPos, float factor)
{
	float dist = length(startPos - uCameraPosition);
	float radius = (uCameraPosition.y - worldSphereCenter.y) * 0.3f;
	float alpha = (dist / radius);
	return (1.0f - exp(-dist * alpha * factor));
}

float GetHeightFraction(vec3 pos)
{
	return (length(pos - worldSphereCenter) - SPHERE_INNER_RADIUS) / (SPHERE_OUTER_RADIUS - SPHERE_INNER_RADIUS);
}

float Remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float RaymarchToLight(vec3 origin, float stepSize, vec3 lightDir, float originalDensity, float lightDotEye)
{
	const float CONE_STEP = 1.0f / 6.0f;

	vec3 startPos = origin;
	float dist = stepSize * 6.0f;
	vec3 rayStep = lightDir * dist;

	float coneRadius = 1.0f;
	float coneDensity = 0.0f;
	float invDepth = 1.0f / dist;
	float sigmaDist = -dist * uAbsorptionToLight;

	float totalDensity = 0.0f;
	float T = 1.0f;
	vec3 marchPos;
	for(int i = 0; i < 6; i++)
	{
		marchPos = startPos + coneRadius * uNoiseKernel[i] * float(i);

		float heightFraction = GetHeightFraction(marchPos);
		if(heightFraction >= 0)
		{
			float cloudDensity = SampleDensityAtPoint(marchPos, totalDensity > 0.3f, i / 16);
			if(cloudDensity > 0.0f) // Some cloud here
			{
				float Ti = exp(cloudDensity * sigmaDist);
				T *= Ti;
				totalDensity += cloudDensity;
			}
		}

		startPos += rayStep; // Move our march forward
		coneRadius += CONE_STEP; // Make our cone bigger
	}

	return T;
}

float SampleDensityAtPoint(vec3 point, bool expensive, float lod)
{
	float heightFraction = GetHeightFraction(point);
	if(heightFraction < 0.0f || heightFraction > 1.0f)  // Too low or too high
	{
		return 0.0f;
	}

	vec3 anim = heightFraction * uWindDirection * CLOUD_TOP_OFFSET + uWindDirection * uTime * uCloudSpeed;
	vec2 uv = point.xz / SPHERE_INNER_RADIUS + 0.5f;
	vec2 movingUV = vec3(point + anim).xz / SPHERE_INNER_RADIUS + 0.5f;

	vec4 noiseSample = textureLod(uCloudTexture, vec3(uv * uCrispiness, heightFraction), lod);
	float lowFreqFBM = dot(noiseSample.gba, vec3(0.625f, 0.25f, 0.125f));
	float baseCloud = Remap(noiseSample.r, -(1.0f - lowFreqFBM), 1.0f, 0.0f, 1.0f);

	// Get density
	float cloudType = 1.0f;
	float stratusFactor = 1.0f - clamp(cloudType * 2.0f, 0.0f, 1.0f);
	float stratoCumulusFactor = 1.0f - abs(cloudType - 0.5f) * 2.0f;
	float cumulusFactor = clamp(cloudType - 0.5f, 0.0f, 1.0f) * 2.0f;

	// Gradient computation (http://advances.realtimerendering.com/s2017/Nubis%20-%20Authoring%20Realtime%20Volumetric%20Cloudscapes%20with%20the%20Decima%20Engine%20-%20Final%20.pdf)
	vec4 baseGradient = stratusFactor * STRATUS_GRADIENT + stratoCumulusFactor * STRATOCUMULUS_GRADIENT + cumulusFactor * CUMULUS_GRADIENT;
	float density = smoothstep(baseGradient.x, baseGradient.y, heightFraction) - smoothstep(baseGradient.z, baseGradient.w, heightFraction);

	baseCloud *= density / heightFraction;

	// Weather/Cloud Coverage
	vec3 weatherSample = texture(uWeatherTexture, movingUV).rgb;
	float cloudCoverage = weatherSample.r * uCloudCoverageMult;
	float baseCloudWithCoverage = Remap(baseCloud, cloudCoverage, 1.0f, 0.0f, 1.0f);
	baseCloudWithCoverage *= cloudCoverage;

	if(expensive) // "expensive" flag is enabled. This will give the clouds more detail from sampling from another noise texture
	{
		vec3 erosionSample = textureLod(uWorleyTexture, vec3(movingUV * uCrispiness, heightFraction) * uDetail, lod).rgb;
		float highFreqFBM = dot(erosionSample, vec3(0.625f, 0.25f, 0.125f));
		float highFreqNoiseModifier = mix(highFreqFBM, 1.0f - highFreqFBM, clamp(heightFraction * 10.0f, 0.0f, 1.0f));

		baseCloudWithCoverage = baseCloudWithCoverage - highFreqNoiseModifier * (1.0f - baseCloudWithCoverage);
		baseCloudWithCoverage = Remap(baseCloudWithCoverage * 2.0f, highFreqNoiseModifier * 0.2f, 1.0f, 0.0f, 1.0f);
	}

	return clamp(baseCloudWithCoverage, 0.0f, 1.0f);
}

float HenyeyGreenstein(float a, float g)
{
	float g2 = g * g;
	return (1.0f - g2) / pow(1.0f + g2 - 2.0f * g * a, 1.5f);
}