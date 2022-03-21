//type vertex
#version 420

layout (location = 0) in vec3 vWorldPosition;

void main()
{	
	gl_Position = vec4(vWorldPosition, 1.0f);
};

//type geometry
#version 420

layout (points, invocations = 1) in; // We only take in a point (the world pos of the grass blade)
layout (triangle_strip, max_vertices = 12) out;

out vec3 mWorldPosition;
out vec3 mNormal;
out vec2 mTextureCoordinates;

uniform vec3 uCameraPosition;
uniform mat4 uProjection;
uniform mat4 uView;

uniform vec2 uWidthHeight; // x = width, y = height

// Wind
uniform vec2 uWindParams; // x = oscillationStength, y = force factor
uniform vec2 uWindDirection;
uniform float uTime;

const float PI = 3.141592f;
const float HALF_PI = 1.57079632679f;
const vec3 NORMAL = vec3(0.0f, 0.0f, 1.0f);
const float OSCILLATE_DELTA = 0.05f;

int fmod(int x, int y); // Returns the remainder of x divided by y with the same sign as x. If y is zero, the result is implementation-defined because of division by zero.

void main()
{
    mat4 viewProj = uProjection * uView;
    vec4 worldPos = gl_in[0].gl_Position;

    int vertexCount;
    float heightFactor;

    // LOD
    float dist = length(uCameraPosition - worldPos.xyz);
    if(dist < 100.0f)
    {
        vertexCount = 12;
        heightFactor = 1.0f;
    }
    else if(dist < 500.0f)
    {
        vertexCount = 8;
        heightFactor = 1.7f;
    }
    else
    {
        vertexCount = 4;
        heightFactor = 4.5f;
    }

    // Generate a random number per-blade to vary width and height
	float random = sin(HALF_PI * fract(worldPos.x) + HALF_PI * fract(worldPos.z)); 

    float width = uWidthHeight.x;
    float height = uWidthHeight.y * heightFactor;

    float currentWidth = 0.0f;
    float currentHeight = 0.0f;
    vec4 vertexPos;

    // Emit start vertex
    gl_Position = viewProj * worldPos;
    EmitVertex();

    float windCoEff = 0.0f;

    for(int i = 0; i < vertexCount - 1; i++)
    {
        // Wind
        vec2 wind = vec2(sin(uTime * PI), sin(uTime * PI));
        wind.x += (sin(uTime + worldPos.x / 25.0f) + sin((uTime + worldPos.x / 15.0f) + 50.0f)) * 0.5f;
        wind.y += cos(uTime + worldPos.z / 80.0f);
        wind *= mix(0.7f, 1.0f, 1.0f - random);

        // Wind oscillation
        float lerpCoeff = (sin(uWindParams.x * uTime + random) + 1.0f) / 2.0f;
        vec2 leftWindBound = wind * (1.0f - OSCILLATE_DELTA);
        vec2 rightWindBound = wind * (1.0f + OSCILLATE_DELTA);
        wind = mix(leftWindBound, rightWindBound, lerpCoeff);

        // Determin wind direction and force
        float randomMagnitude = mix(0.0f, 1.0f, random);
        wind += uWindDirection * randomMagnitude;
        wind *= uWindParams.y;
        float windForce = length(wind);

        if(i % 2 == 0) // Moving horizontal
        {
            vertexPos = vec4(worldPos.x + width, worldPos.y + currentHeight, worldPos.z, 1.0f);
        }
        else // Moving up
        {
            currentHeight += height;
            windCoEff += height;
            vertexPos = vec4(worldPos.x, worldPos.y + currentHeight, worldPos.z, 1.0f);
        }    

        vertexPos.xz += wind * windCoEff;
        //vertexPos.y -= windForce * windCoEff * 0.8f;

        gl_Position = viewProj * vertexPos;
        EmitVertex();
    }
    EndPrimitive();
}

int fmod(int x, int y)
{
	float remainder = fract(abs(x / y)) * abs(y);
	return int(x < 0.0f ? -remainder : remainder);
}

//type fragment
#version 420 

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec3 gEffects;

in vec3 mWorldPosition;
in vec3 mNormal;
in vec2 mTextureCoordinates;

out vec4 oColor;

uniform sampler2D uAlbedoTexture;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

// Material
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

uniform sampler2D uDiscardTexture;

const float nearPlane = 0.1f;
const float farPlane = 1000.0f;

vec3 LinearizeColor(vec3 color); // Converts to linear color space (sRGB to RGB). In other words, gamma correction https://lettier.github.io/3d-game-shaders-for-beginners/gamma-correction.html
float LinearizeDepth(float depth);
vec3 ComputeTextureNormal();

void main()
{
//	if(texture(uDiscardTexture, mTextureCoordinates).r <= 0.0f) // Shouldn't draw pixel here
//	{
//		discard;
//		return;
//	}
//	
//	if(uHasNormalTexture)
//	{		
//		gNormal.rgb = ComputeTextureNormal(); // Assign normal
//	}
//	else
//	{
//		gNormal.rgb = mNormal;
//	}
//	
//	gPosition = vec4(mWorldPosition, LinearizeDepth(gl_FragCoord.z)); // Set position with adjusted depth
//	gAlbedo.rgb = texture(uAlbedoTexture, mTextureCoordinates).rgb;
//	gEffects.gb = vec2(0.0f, 0.0f);
//	
//	if(uMaterialOverrides.w == 1.0f) // No texture to sample from, use flat value
//	{
//		gAlbedo.a = uMaterialOverrides.r; // Roughness
//		gNormal.a = uMaterialOverrides.g; // Metalness
//		gEffects.r = uMaterialOverrides.b; // AO
//	}
//	else
//	{
//		gAlbedo.a = vec3(texture(uRoughnessTexture, mTextureCoordinates)).r; // Sample and assign roughness value
//		gNormal.a = vec3(texture(uMetalnessTexture, mTextureCoordinates)).r; // Sample and assign metalness value
//		gEffects.r = vec3(texture(uAmbientOcculsionTexture, mTextureCoordinates)).r;
//	}

    oColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

vec3 LinearizeColor(vec3 color)
{
	return pow(color.rgb, vec3(2.2f));
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

vec3 ComputeTextureNormal()
{
	vec3 textureNormal = normalize(texture(uNormalTexture, mTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
	
	// Get partial derivatives 
    vec3 dPosX = dFdx(mWorldPosition);
    vec3 dPosY = dFdy(mWorldPosition);
    vec2 dTexX = dFdx(mTextureCoordinates);
    vec2 dTexY = dFdy(mTextureCoordinates);

	// Convert normal to tangent space
    vec3 normal = normalize(mNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * textureNormal);
}