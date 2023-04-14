#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;


uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * vec4(aPosition, 1.0);
}


#shader fragment
#version 450 core

uniform vec3 u_Color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(u_Color, 1.0);
}