//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;

uniform mat4 uMatModel;

void main()
{	
	gl_Position = uMatModel * vec4(vPosition, 1.0f);
};


//type geometry
#version 420

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform uLightSpaceMatrices // Our UniformBuffer from our cpp code (found in CascadedShadowMapping.h)
{
	mat4 lightSpaceMatrices[16];
};

void main()
{ 
	for(int i = 0; i < 3; i++)
	{
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}


//type fragment
#version 420 

uniform float uShadowSoftness;

out vec4 oColor;

void main()
{
	oColor = vec4(uShadowSoftness, uShadowSoftness, uShadowSoftness, 1.0f);
}