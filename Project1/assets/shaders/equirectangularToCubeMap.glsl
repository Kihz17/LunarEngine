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

uniform sampler2D uEnvMap;

const float PI  = 3.14159265359f;

vec2 ConvertCoordsToSpherical(vec3 normalizedCoords); 

void main()
{
	vec2 textureCoords = ConvertCoordsToSpherical(normalize(mLocalPosition));
	vec3 color = texture(uEnvMap, textureCoords).rgb;
	oColor = vec4(color, 1.0f);
}

vec2 ConvertCoordsToSpherical(vec3 normalizedCoords)
{
    float phi = acos(-normalizedCoords.y);
    float theta = atan(1.0f * normalizedCoords.x, -normalizedCoords.z) + PI;

    return vec2(theta / (2.0f * PI), phi / PI);
}