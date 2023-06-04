#shader vertex
#version 450 core

layout(location = 0) in vec2 InPosition;
layout(location = 1) in vec3 InColor;


layout(location = 0) out vec3 VertexColor;

void main()
{
    gl_Position = vec4(InPosition, 0.0, 1.0);
    VertexColor = InColor;
}


#shader fragment
#version 450 core

layout(location = 0) in vec3 VertexColor;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(VertexColor, 1.0);
}