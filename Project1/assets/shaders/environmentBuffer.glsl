//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTextureCoordinates;

out vec3 mWorldPosition;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{	
	mWorldPosition = vPosition;
	mat4 rotationView = mat4(mat3(uView));
	vec4 clipPosition = uProjection * rotationView * vec4(mWorldPosition, 1.0f);
	gl_Position  = clipPosition.xyww;
};



//type fragment
#version 420 

layout (location = 0) out vec4 oColor;

in vec3 mWorldPosition;

uniform samplerCube uEnvMap;

uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform vec2 uResolution;
uniform vec4 uLightDirection;
uniform vec3 uLightColor;

vec3 ComputeClipSpaceCoord(ivec2 fragCoord);

void main()
{
	ivec2 fragCoord = ivec2(gl_FragCoord.xy);
	vec3 envColor = textureLod(uEnvMap, mWorldPosition, 0.0).rgb;
	
	if(uLightDirection.w == 1.0f) // Create sun
	{
		// Create ray
		vec4 rayClip = vec4(ComputeClipSpaceCoord(fragCoord), 1.0f);
		vec4 rayView = uInvProj * rayClip;
		rayView = vec4(rayView.xy, -1.0f, 0.0f);
		vec3 rayDir = normalize(vec3(uInvView * rayView));

		// Create sun
		float sun = clamp(dot(uLightDirection.xyz, rayDir), 0.0f, 1.0f);
		vec3 sunColor = uLightColor * pow(sun, 350.0f);
		envColor += sunColor;
	}

	// Tonemap and gamma correciton
	//envColor = envColor / (envColor + vec3(1.0f));
	//envColor = pow(envColor, vec3(1.0f / 2.2f));
	
	oColor = vec4(envColor, 1.0f);
}

vec3 ComputeClipSpaceCoord(ivec2 fragCoord)
{
	vec2 rayNDC = 2.0f * vec2(fragCoord.xy) / uResolution.xy - 1.0f;
	return vec3(rayNDC, 1.0f);
}
