//type vertex
#version 420

layout (location = 0) in vec3 vPosition;

uniform mat4 uMatModel;
uniform mat4 uMatView;
uniform mat4 uMatProjection;

void main()
{		
	gl_Position = uMatProjection * uMatView * uMatModel * vec4(vPosition, 1.0f);
};



//type fragment
#version 420 

out vec4 oColor;

uniform vec3 uLineColor;

void main()
{
	oColor = vec4(uLineColor, 1.0f);
}