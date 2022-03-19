//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;
layout (location = 3) in vec4 vBoneIDs;
layout (location = 4) in vec4 vBoneWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 uMatModel;
uniform mat4 uBoneMatrices[MAX_BONES];

void main()
{	
	vec4 vertexPos = vec4(vPosition, 1.0f);

	vec4 transformedPos = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		int boneID = int(vBoneIDs[i]);
		if(boneID == -1) break; // Bone ID was still non-existent, nothing to do here
		
		if(boneID >= MAX_BONES) // We have exceeded the max bone count, just set this vertex to the default vertex position
		{
			transformedPos = vertexPos;
			break;
		}
		
		float weight = vBoneWeights[i];

		vec4 localPos = uBoneMatrices[boneID] * vertexPos;
		transformedPos += localPos * weight;
	}
	
	gl_Position = uMatModel * transformedPos;
};


//type geometry
#version 420

layout(triangles, invocations = 6) in; // NOTE: NUMBER OF INVOCATIONS MUST MATCH THE NUMBER OF CASCADE LAYERS
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

void main()
{
	// Pass through
}