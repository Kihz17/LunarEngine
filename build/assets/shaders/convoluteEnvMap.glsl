//type vertex
#version 420

layout (location = 0) in vec3 vPosition;

out vec3 mLocalPosition;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{	
	mLocalPosition = vPosition;
	gl_Position = uProjection * uView * vec4(mLocalPosition, 1.0f);
};



//type fragment
#version 420 

in vec3 mLocalPosition;

out vec4 oColor;

uniform samplerCube uEnvMap;

const float PI  = 3.14159265359f;

void main()
{
	vec3 normal = normalize(mLocalPosition);
	vec3 irradiance = vec3(0.0f);
	
	vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = normalize(cross(worldUp, normal));
	worldUp = normalize(cross(normal, right));
	
	float sampleOffset = 0.025f;
	float sampleCount = 0.0f;
	for(float phi = 0.0f; phi < 2.0f * PI; phi += sampleOffset)
	{
		for(float theta = 0.0f; theta < 0.5f * PI; theta += sampleOffset)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)); // Spherical to cartesian (in tangent space)
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * worldUp + tangentSample.z * normal; // Tangent space to world space
			
			irradiance += texture(uEnvMap, sampleVec).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}
	
	irradiance = PI * irradiance * (1.0f / float(sampleCount));
	
	oColor = vec4(irradiance, 1.0f);
}