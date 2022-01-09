//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoordinates;

out vec2 mTextureCoordinates;

void main()
{	
	mTextureCoordinates = vTextureCoordinates;
	gl_Position = vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

in vec2 mTextureCoordinates;

out vec2 oColor;

const float PI = 3.14159265359f;
const uint SAMPLE_COUNT = 1024u;

float RadicalInverse_VdC(uint bits); // Random number generator
vec2 ComputeHammersley(uint i, uint N); // Gives us low descrepancy sample of i of the total sample set of N
vec3 ComputeImportanceSampleGGX(vec2 Xi, vec3 N, float roughness); // Generates a sample vector biased towards the general reflection orientation of the microsurface halway vector based on the surface's roughness
float ComputeDistributionGGX(vec3 N, vec3 H, float roughness);
float ComputeGeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness);
vec2 ComputeIntegrateBRDF(float NdotV, float roughness);

void main()
{
	vec2 integratedBRDF = ComputeIntegrateBRDF(mTextureCoordinates.x, mTextureCoordinates.y);
	oColor = integratedBRDF;
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

float ComputeGeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness + 0.0001f;

    float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
    float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

    return ggxL * ggxV;
}

vec2 ComputeIntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    vec3 N = vec3(0.0f, 0.0f, 1.0f);
    vec2 brdfLUT = vec2(0.0f);

    for(uint i = 0; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = ComputeHammersley(i, SAMPLE_COUNT);
        vec3 H = ComputeImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = clamp(L.z, 0.0f, 1.0f);
        float NdotH = clamp(H.z, 0.0f, 1.0f);
        float VdotH = clamp(dot(V, H), 0.0f, 1.0f);

        if(NdotL > 0.0f)
        {
            float G = ComputeGeometryAttenuationGGXSmith(NdotL, NdotV, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            brdfLUT.x += (1.0f - Fc) * G_Vis;
            brdfLUT.y += Fc * G_Vis;
        }
    }

    return brdfLUT / SAMPLE_COUNT;
}
// RESOURCES
// https://learnopengl.com/PBR/IBL/Specular-IBL
// https://chetanjags.wordpress.com/2015/08/26/image-based-lighting/