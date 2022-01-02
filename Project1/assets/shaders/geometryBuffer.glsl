//type vertex
#version 420

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoordinates;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;
uniform mat4 uMatProjViewModel;
uniform mat4 uMatPrevProjViewModel;

out vec3 oViewPosition;
out vec2 oTextureCoordinates;
out vec3 oNormal;
out vec4 oFragPosition;
out vec4 oPrevFragPosition;

void main()
{	
	// Translate to view space
	vec4 viewFragmentPosition = uMatView * uMatModel * vec4(vPosition, 1.0f);
	oViewPosition = viewFragmentPosition.xyz;
	
	oTextureCoordinates = vTextureCoordinates;
	
	// Apply transformation to normal
	mat3 matNormal = transpose(inverse(mat3(uMatView * uMatModel)));
	oNormal = matNormal * vNormal;
	
	oFragPosition = uMatProjViewModel * vec4(vPosition, 1.0f);
	oPrevFragPosition = uMatPrevProjViewModel * vec4(vPosition, 1.0f);
	
	gl_Position = uMatProjection * viewFragmentPosition;
};



//type fragment
#version 420 

layout (location = 0) out vec4 vPosition;
layout (location = 1) out vec4 vAlbedo;
layout (location = 2) out vec4 vNormal;
layout (location = 3) out vec3 vEffects;

in vec3 iViewPosition;
in vec2 iTextureCoordinates;
in vec3 iNormal;
in vec4 iFragPosition;
in vec4 iPrevFragPosition;

uniform vec3 uAlbedoColor;
uniform sampler2D uAlbedoTexture1;
uniform sampler2D uAlbedoTexture2;
uniform sampler2D uAlbedoTexture3;
uniform sampler2D uAlbedoTexture4;

uniform sampler2D uNormalTexture;
uniform sampler2D uRoughnessTexture;
uniform sampler2D uMetalnessTexture;
uniform sampler2D uAmbientOcculsionTexture;

const float nearPlane = 1.0f;
const float farPlane = 1000.0f;

float LinearizeDepth(float depth);
vec3 ComputeTextureNormal(vec3 viewNormal, vec3 textureNormal);

void main()
{
	vec3 normal = normalize(texture(uNormalTexture, iTextureCoordinates).rgb * 2.0f - 1.0f); // Sample normal texture and convert values in range from -1.0 to 1.0
	
	vec2 fragPos = (iFragPosition.xy / iFragPosition.w) * 0.5f + 0.5f;
	vec2 prevFragPos = (iPrevFragPosition.xy / iPrevFragPosition.w) * 0.5f + 0.5f;
	
	vPosition = vec4(iViewPosition, LinearizeDepth(gl_FragCoord.z)); // Set position with adjusted depth
	
	vAlbedo.rgb = vec3(texture(uAlbedoTexture1, iTextureCoordinates)); // Sample and assign albedo rgb colors
	vAlbedo.rgb += vec3(texture(uAlbedoTexture2, iTextureCoordinates));
	vAlbedo.rgb += vec3(texture(uAlbedoTexture3, iTextureCoordinates)); 
	vAlbedo.rgb += vec3(texture(uAlbedoTexture4, iTextureCoordinates)); 
	
	vAlbedo.a = vec3(texture(uRoughnessTexture, iTextureCoordinates)).r; // Sample and assign roughness value
	
	vNormal.rgb = ComputeTextureNormal(iNormal, normal); // Assign normal
	vNormal.a = vec3(texture(uMetalnessTexture, iTextureCoordinates)).r; // Sample and assign metalness value
	
	vEffects.r = vec3(texture(uAmbientOcculsionTexture, iTextureCoordinates)).r;
	vEffects.gb = fragPos - prevFragPos;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

vec3 ComputeTextureNormal(vec3 viewNormal, vec3 textureNormal)
{
	// Get partial derivatives 
    vec3 dPosX = dFdx(iViewPosition);
    vec3 dPosY  = dFdy(iViewPosition);
    vec2 dTexX = dFdx(iTextureCoordinates);
    vec2 dTexY = dFdy(iTextureCoordinates);

	// Convert normal to tangent space
    vec3 normal = normalize(viewNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * textureNormal);
}