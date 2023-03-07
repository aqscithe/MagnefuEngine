#shader vertex
#version 450 core

layout(location = 0) in vec4 position;


void main()
{
	gl_Position = position;
}


#shader fragment
#version 450 core

out vec4 fragColor;

uniform vec4 u_Color;

void main()
{
	fragColor = u_Color;
}