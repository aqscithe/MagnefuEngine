#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoords;

out vec4 VertexColor;
out vec2 TexCoords;

uniform mat4 u_MVP;


void main()
{
	// Must do multiplication in reverse order as I am using row major ordering
	gl_Position = u_MVP * vec4(aPosition, 1.0);
	VertexColor = vec4(aColor, 1.0);
	TexCoords = aTexCoords;
}



#shader fragment
#version 450 core


in vec4 VertexColor;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D u_DiffuseTexture;

void main()
{
	FragColor = texture(u_DiffuseTexture, TexCoords) * VertexColor;
	//vec4 testColor = vec4(0.1, 0.7, 0.9, 0.2);
	//FragColor = texture(u_DiffuseTexture, TexCoords) * testColor;
}