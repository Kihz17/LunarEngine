#version 430

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba8, binding = 0) uniform image2D uWeatherTexture;

// Adapted From https://github.com/sebh/TileableVolumeNoise
uniform float frequenceMul[6u] = float[]( 2.0,8.0,14.0,20.0,26.0,32.0 );

uniform vec2 uResolution;
uniform vec3 uSeed;
uniform float uPerlinAmplitude = 0.5f;
uniform float uPerlinFrequency = 0.8f;
uniform float uPerlinScale = 100.0f;

float Random2D(in vec2 st);
float NoiseInterpolation(in vec2 i_coord, in float i_size);
float PerlinNoise(vec2 uv, float sc, float f, float a, int o);

void main()
{
    ivec2 fragPos = ivec2(gl_GlobalInvocationID.xy);

	vec2 uv = vec2(float(fragPos.x + 2.0f) / uResolution.x, float(fragPos.y) / uResolution.y);
	vec2 suv = vec2(uv.x + 5.5f, uv.y + 5.5f);

    float cloudType = clamp(PerlinNoise(suv, uPerlinScale * 3.0f, 0.3f, 0.7f, 10), 0.0f, 1.0f);

    float coverage = PerlinNoise(uv, uPerlinScale * 0.95f, uPerlinFrequency, uPerlinAmplitude, 4);
    vec4 weather = vec4(clamp(coverage, 0.0f, 1.0f), cloudType, 0.0f, 1.0f);

    imageStore(uWeatherTexture, fragPos, weather);
}

float Random2D(in vec2 st) 
{
    return fract( sin(dot( st.xy, vec2(12.9898,78.233 ) + uSeed.xy ) ) * 43758.5453123);
}

float NoiseInterpolation(in vec2 uv, in float size)
{
	vec2 grid = uv * size;
    
    vec2 randomInput = floor(grid);
    vec2 weights = fract(grid);
    
    float p0 = Random2D(randomInput);
    float p1 = Random2D(randomInput + vec2(1.0f, 0.0f));
    float p2 = Random2D(randomInput + vec2(0.0f, 1.0f));
    float p3 = Random2D(randomInput + vec2(1.0f, 1.0f));
    
    weights = smoothstep( vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), weights ); 
    
    return p0 +
           (p1 - p0) * (weights.x) +
           (p2 - p0) * (weights.y) * (1.0f - weights.x) +
           (p3 - p1) * (weights.y * weights.x);    
}

float PerlinNoise(vec2 uv, float sc, float f, float a, int o)
{
    float noiseValue = 0.0;
    
    float localAplitude  = a;
    float localFrecuency = f;

    for( int index = 0; index < o; index++ )
    {
     	       
        noiseValue += NoiseInterpolation( uv, sc * localFrecuency ) * localAplitude;
    
        localAplitude   *= 0.25;
        localFrecuency  *= 3.0;
    }    

	return noiseValue * noiseValue;
}