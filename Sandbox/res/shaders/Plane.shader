#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTexCoords;


uniform mat4 u_MVP;

out vec3 VertexColor;
void main()
{
	gl_Position = u_MVP * vec4(aPosition, 1.0);
	VertexColor = aColor;
}


#shader fragment
#version 450 core

in vec3 VertexColor;
out vec4 fragColor;

void main()
{
	fragColor = vec4(VertexColor, 1.0);
}