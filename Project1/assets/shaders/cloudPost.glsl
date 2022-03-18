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

uniform sampler2D uCloudsTexture;
uniform sampler2D uEmissionTexture;

uniform bool uEnableGodRays;
uniform vec3 uLightPosition;
uniform float uLightDotCameraDir;

uniform vec4 uRadialBlurParams; // x = decay, y = density, z = weight, w = exposure

uniform vec2 uResolution;
uniform vec2 uCloudResolution;

const float FAR_PLANE = 1000.0f;
const float NEAR_PLANE = 0.1f;

vec2 offset =  1.0f / uCloudResolution;
const vec2 offsets1[9] = vec2[]
(
	vec2(-offset.x, offset.y), // top-left
    vec2(0.0f, offset.y), // top-center
    vec2(offset.x, offset.y), // top-right
    vec2(-offset.x, 0.0f),   // center-left
    vec2(0.0f, 0.0f),   // center-center
    vec2(offset.x, 0.0f),   // center-right
    vec2(-offset.x, -offset.y), // bottom-left
    vec2(0.0f, -offset.y), // bottom-center
    vec2(offset.x, -offset.y)  // bottom-right   
);

const vec2 offsets2[18] = vec2[]
(
	vec2(-offset.x * 2.0f, offset.y * 2.0f), 
	vec2(-offset.x, offset.y), 

	vec2(0.0f, offset.y * 2.0f), 
	vec2(0.0f, offset.y), 

	vec2(offset.x* 2.0f, offset.y* 2.0f), 
	vec2(offset.x, offset.y), 
 
	vec2(-offset.x * 2.0f, 0.0f),   
	vec2(-offset.x, 0.0f),   
  
	vec2(0.0f, 0.0f),
	vec2(0.0f, 0.0f),

	vec2(offset.x * 2.0f, 0.0f),  
	vec2(offset.x, 0.0f),  

	vec2(-offset.x * 2.0f, -offset.y * 2.0f), 
	vec2(-offset.x, -offset.y), 

	vec2(0.0f, -offset.y * 2.0f),
	vec2(0.0f, -offset.y), 

	vec2(offset.x * 2.0f, -offset.y * 2.0f),
	vec2(offset.x, -offset.y)
);

const vec2 offsets3[27] = vec2[]
(
	vec2(-offset.x * 3.0f, offset.y * 3.0f), 
	vec2(-offset.x * 2.0f, offset.y * 2.0f), 
	vec2(-offset.x, offset.y), 

	vec2(0.0f, offset.y * 3.0f), 
	vec2(0.0f, offset.y * 2.0f), 
	vec2(0.0f, offset.y), 

	vec2(offset.x* 3.0f, offset.y* 3.0f), 
	vec2(offset.x* 2.0f, offset.y* 2.0f), 
	vec2(offset.x, offset.y), 

	vec2(-offset.x * 3.0f, 0.0f),   
	vec2(-offset.x * 2.0f, 0.0f),   
	vec2(-offset.x, 0.0f),   

	vec2(0.0f, 0.0f),   
	vec2(0.0f, 0.0f),
	vec2(0.0f, 0.0f),

	vec2(offset.x * 3.0f, 0.0f),  
	vec2(offset.x * 2.0f, 0.0f),  
	vec2(offset.x, 0.0f),  

	vec2(-offset.x * 3.0f, -offset.y * 3.0f), 
	vec2(-offset.x * 2.0f, -offset.y * 2.0f), 
	vec2(-offset.x, -offset.y), 

	vec2(0.0f, -offset.y * 3.0f),
	vec2(0.0f, -offset.y * 2.0f),
	vec2(0.0f, -offset.y), 

	vec2(offset.x * 3.0f, -offset.y * 3.0f),
	vec2(offset.x * 2.0f, -offset.y * 2.0f),
	vec2(offset.x, -offset.y)
);

const float kernel1[9] = float[]
(
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16  
);

const float kernel2[18] = float[]
(
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,

	4.0 / 16, 6.0 / 16, 4.0 / 16,
	4.0 / 16, 6.0 / 16, 4.0 / 16,

	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16
);

const float kernel3[27] = float[]
(
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	3.0 / 16, 8.0 / 16, 3.0 / 16,

	4.0 / 16, 8.0 / 16, 4.0 / 16,
	5.0 / 16, 10.0 / 16, 5.0 / 16,
	4.0 / 16, 8.0 / 16, 4.0 / 16 ,

	3.0 / 16, 6.0 / 16, 3.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16
);

vec4 GaussianBlur(sampler2D t, vec2 uv);
vec4 SmoothCloudMedium();
vec4 SmoothCloudHigh();

void main()
{
	oColor = GaussianBlur(uCloudsTexture, mTextureCoordinates);

	// God rays
	if(uLightDotCameraDir > 0.0f && uEnableGodRays)
	{
		vec2 uv = mTextureCoordinates;

		const int numSamples = 64;

		vec2 deltaUV = uv - uLightPosition.xy;
		deltaUV *= uRadialBlurParams.y / float(numSamples);

		float illuminationDecay = 1.0f;

		vec3 rayColor = GaussianBlur(uEmissionTexture, mTextureCoordinates).rgb * 0.4f;

		for(int i = 0; i < numSamples; i++)
		{
			uv -= deltaUV;
			rayColor += texture(uEmissionTexture, uv).rgb * illuminationDecay * uRadialBlurParams.z;
			illuminationDecay *= uRadialBlurParams.x;
		}

		// Mix ray color with sky
		vec3 colorWithRay = oColor.rgb + (smoothstep(0.0f, 1.0f, rayColor) * uRadialBlurParams.w);
		oColor.rgb = mix(oColor.rgb, colorWithRay * 0.9f, uLightDotCameraDir * uLightDotCameraDir);
	}
}

vec4 GaussianBlur(sampler2D t, vec2 uv)
{
	// Sample neighbouring 9 pixels
	vec4 sampledValues[9];
	for(int i = 0; i < 9; i++)
	{
		sampledValues[i] = texture(t, uv + offsets1[i]);
	}

	// Blur between all sampled values
	vec4 color = vec4(0.0f);
	for(int i = 0; i < 9; i++)
	{
		color += sampledValues[i] * kernel1[i];
	}

	return color;
}

vec4 SmoothCloudMedium()
{
	// Sample neighbouring 18 pixels
	vec4 sampledValues[18];
	for(int i = 0; i < 18; i++)
	{
		sampledValues[i] = texture(uCloudsTexture, mTextureCoordinates + offsets2[i]);
	}

	// Blur between all sampled values
	vec4 color = vec4(0.0f);
	for(int i = 0; i < 18; i++)
	{
		color += sampledValues[i] * kernel2[i];
	}

	return color;
}

vec4 SmoothCloudHigh()
{
	// Sample neighbouring 27 pixels
	vec4 sampledValues[27];
	for(int i = 0; i < 27; i++)
	{
		sampledValues[i] = texture(uCloudsTexture, mTextureCoordinates + offsets3[i]);
	}

	// Blur between all sampled values
	vec4 color = vec4(0.0f);
	for(int i = 0; i < 27; i++)
	{
		color += sampledValues[i] * kernel3[i];
	}

	return color;
}