//type vertex
#version 420

layout (location = 0) in vec3 vPosition;

out vec3 mLocalPosition;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{	
	mLocalPosition = vPosition;
	gl_Position = uProjection * mat4(mat3(uView)) * vec4(mLocalPosition, 1.0f);
};



//type fragment
#version 420 

in vec3 mLocalPosition;

out vec4 oColor;

uniform samplerCube uEnvMap;
uniform float uRoughness;
uniform vec2 uCubeResolution; // Per-face resolution

const float PI = 3.14159265359f;
const uint SAMPLE_COUNT = 1024u;

float RadicalInverse_VdC(uint bits); // Random number generator
vec2 ComputeHammersley(uint i, uint N); // Gives us low descrepancy sample of i of the total sample set of N
vec3 ComputeImportanceSampleGGX(vec2 Xi, vec3 N, float roughness); // Generates a sample vector biased towards the general reflection orientation of the microsurface halway vector based on the surface's roughness
float ComputeDistributionGGX(vec3 N, vec3 H, float roughness);

void main()
{
	vec3 normal = normalize(mLocalPosition);
	
	// UE4 "Real Shading" approximation
	vec3 R = normal;
	vec3 V = R;
	
	vec3 prefilteredColor = vec3(0.0f);
	float sampleWeight = 0.0f;
	
	for(uint i = 0u; i < SAMPLE_COUNT; i++)
	{
		vec2 Xi = ComputeHammersley(i, SAMPLE_COUNT);
		vec3 H = ComputeImportanceSampleGGX(Xi, normal, uRoughness); // Generate "semi-random" halfway vector using the Hammersley sequence (generates random points that a reasonably spread out and not clustered) https://gyazo.com/c2d716a29911dbf5cfa14e4f1e4fd206
		vec3 L = normalize(2.0f * dot(V, H) * H - V); // Vector to the light
		
		float NdotL = max(dot(normal, L), 0.0f); // Intensity of normal and light direction

		if(NdotL > 0.0f)
		{
			// Determine mip level from roughness
			float D = ComputeDistributionGGX(normal, H, uRoughness);
			float NdotH = max(dot(normal, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);
			float probabilityDistribution = D * NdotH / (4.0f * HdotV) + 0.0001f;
			
			// Trick to avoid render artifacts like easily dotted patterns around bright areas https://gyazo.com/9499c989d8866a87f4909e16eaa21bb8
			float saTexel = 4.0f * PI / (6.0f * uCubeResolution.x * uCubeResolution.y);
			float saSample = 1.0f / (float(SAMPLE_COUNT) * probabilityDistribution + 0.0001f);
			float mipLevel = uRoughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);
			
			prefilteredColor += textureLod(uEnvMap, L, mipLevel).rgb * NdotL; // Sample texel at given mip level in direction of light scaled by the intensity of the light angle dotted by normal
			sampleWeight += NdotL; // Incrememt weight by the intensity of the light angle dotted by normal
		}
	}
	
	prefilteredColor = prefilteredColor / sampleWeight;
	oColor = vec4(prefilteredColor, 1.0f);
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10;
}

vec2 ComputeHammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ComputeImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float alpha = roughness * roughness;

    float anglePhi = 2 * PI * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (alpha * alpha - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    vec3 H;
    H.x = sinTheta * cos(anglePhi);
    H.y = sinTheta * sin(anglePhi);
    H.z = cosTheta;

    vec3 upDir = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 tanX = normalize(cross(upDir, N));
    vec3 tanY = cross(N, tanX);

    return normalize(tanX * H.x + tanY * H.y + N * H.z);
}

float ComputeDistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH = clamp(dot(N, H), 0.0f, 1.0f);
    float NdotH2 = NdotH * NdotH;

    return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0f) + 1.0f) * (NdotH2 * (alpha2 - 1.0f) + 1.0f));
}

// RESOURCES
// https://learnopengl.com/PBR/IBL/Specular-IBL
// https://chetanjags.wordpress.com/2015/08/26/image-based-lighting/