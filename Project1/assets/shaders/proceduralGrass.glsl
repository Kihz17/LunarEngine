//type vertex
#version 420

layout (location = 0) in vec4 vWorldPosition; // w represents rotation around y axis

out mat4 mRotationMatrix;

void main()
{	
	gl_Position = vWorldPosition;
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
uniform vec3 uWindParams; // x = oscillationStength, y = force factor, z = stiffness
uniform vec2 uWindDirection;
uniform float uTime;

// Terrain
uniform vec2 uSeed;
uniform vec4 uTerrainParams; // x = amplitude, y = roughness, z = persistence, w = freq
uniform int uOctaves;

const float PI = 3.141592f;
const float HALF_PI = 1.57079632679f;
const vec3 NORMAL = vec3(0.0f, 0.0f, 1.0f);
const float OSCILLATE_DELTA = 0.05f;

mat4 RotationMatrix(vec3 axis, float angle);
float Rand(vec2 co); // Generates a pseudo random number https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float SmoothNoise(int x, int y); // Generates a pseudo random "smoothed" noise value
float InterpolateCos(float a, float b, float blend); // Cosine interpolation for 2 values
float InterpolateNoise(vec2 v);
float GetHeight(vec2 v);

void main()
{
    mat4 viewProj = uProjection * uView;

    // Get position relative to origin
    vec4 pointPos = vec4(gl_in[0].gl_Position.xyz, 1.0f);

    // Get position in world space
    float terrainHeight = GetHeight(pointPos.xz);
    vec4 worldPos = pointPos + vec4(uCameraPosition.x, terrainHeight, uCameraPosition.z, 1.0f);

    mat4 rotationMat = RotationMatrix(vec3(0.0f, 1.0f, 0.0f), gl_in[0].gl_Position.w);
    vec4 normal = vec4(0.0f, 0.0f, 1.0f, 1.0f);

    int vertexCount;
    float heightFactor;

    // LOD
    float dist = 1.0f; //length(uCameraPosition - worldPos.xyz);
    if(dist < 300.0f)
    {
        vertexCount = 12;
        heightFactor = 1.0f;
    }
    else if(dist < 700.0f)
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
	float random = sin(HALF_PI * fract(pointPos.x) + HALF_PI * fract(pointPos.z)); 

    // Grass dimensions
    const float width = uWidthHeight.x;
    const float height = uWidthHeight.y * heightFactor;
    const float texYIncrement = 1.0f / (vertexCount / 2);

    vec4 vertexPos;

    float currentHeight = 0.0f;
    float windCoEff = 0.0f;
    int stiffnessFactor = 1;
    float texY = 0.0f;

    for(int i = 0; i < vertexCount; i++)
    {
        // Wind
        vec2 wind = vec2(sin(uTime * PI), sin(uTime * PI));
        wind.x += (sin(uTime + pointPos.x / 25.0f) + sin((uTime + pointPos.x / 15.0f) + 50.0f)) * 0.5f;
        wind.y += cos(uTime + pointPos.z / 80.0f);
        wind *= mix(0.7f, 1.0f, 1.0f - random);

        // Wind oscillation
        float lerpCoeff = (sin(uWindParams.x * uTime + random) + 1.0f) / 2.0f;
        vec2 leftWindBound = wind * (1.0f - OSCILLATE_DELTA);
        vec2 rightWindBound = wind * (1.0f + OSCILLATE_DELTA);
        wind = mix(leftWindBound, rightWindBound, lerpCoeff);

        // Determine wind direction and force
        float randomMagnitude = mix(0.0f, 1.0f, random);
        wind += uWindDirection * randomMagnitude;
        wind *= uWindParams.y;
        float windForce = length(wind);

        if(i % 2 == 0) // Moving up
        {
            currentHeight += height;
            windCoEff += height;
            stiffnessFactor++;
            vertexPos = vec4(0.0f, currentHeight, 0.0f, 1.0f);
            texY += texYIncrement;
            mTextureCoordinates = vec2(0.0f, texY);
        }
        else  // Moving horizontal
        {
            vertexPos = vec4(width, currentHeight, 0.0f, 1.0f);   
            mTextureCoordinates = vec2(1.0f, texY);
        }    

        float stiffnessMult = pow(uWindParams.z, vertexCount - stiffnessFactor); // Multiplier to "curve" the grass blades

        // Apply wind to vertex
        vec3 windTransformation = vec3(wind.x, 0.0f, wind.y) * windCoEff * stiffnessMult;
        vertexPos.xyz = (rotationMat * vertexPos).xyz; // Rotate by angle
        vertexPos.xyz += windTransformation; // Transform by wind
        vertexPos.xyz += worldPos.xyz; // Move from origin -> position in world

        mNormal = normal.xyz;
        mWorldPosition = vertexPos.xyz;
        gl_Position = viewProj * vertexPos;

        EmitVertex();
    }
    EndPrimitive();
}

// https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 RotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
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
	vec2 pos = uCameraPosition.xz + v;
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

in vec3 mWorldPosition;
in vec3 mNormal;
in vec2 mTextureCoordinates;

//out vec4 oColor;

uniform sampler2D uAlbedoTexture;

uniform bool uHasNormalTexture;
uniform sampler2D uNormalTexture;

// Material
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform vec4 uMaterialOverrides; // r = roughness, g = metalness, b = ao, w = isMaterialOverride

uniform sampler2D uDiscardTexture;

const float nearPlane = 0.1f;
const float farPlane = 1000.0f;

float LinearizeDepth(float depth);
vec3 ComputeTextureNormal();

void main()
{
	if(texture(uDiscardTexture, mTextureCoordinates).r <= 0.1f) // Shouldn't draw pixel here
	{
		discard;
		return;
	}
	
	if(uHasNormalTexture)
	{		
		gNormal.rgb = ComputeTextureNormal(); // Assign normal
	}
	else
	{
		gNormal.rgb = mNormal;
	}
	
	gPosition = vec4(mWorldPosition, LinearizeDepth(gl_FragCoord.z)); // Set position with adjusted depth
	gAlbedo.rgb = texture(uAlbedoTexture, mTextureCoordinates).rgb;
	gEffects.gb = vec2(0.0f, 0.0f);
	
	if(uMaterialOverrides.w == 1.0f) // No texture to sample from, use flat value
	{
		gAlbedo.a = uMaterialOverrides.r; // Roughness
		gNormal.a = uMaterialOverrides.g; // Metalness
		gEffects.r = uMaterialOverrides.b; // AO
	}
	else
	{
		gAlbedo.a = vec3(texture(uRoughnessTexture, mTextureCoordinates)).r; // Sample and assign roughness value
		gNormal.a = vec3(texture(uMetalnessTexture, mTextureCoordinates)).r; // Sample and assign metalness value
		gEffects.r = 1.0f;
	}
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