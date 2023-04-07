#shader vertex
#version 450 core

layout(location = 0) in vec3 position;


void main()
{
	gl_Position = vec4(position, 1.0);
}


#shader fragment
#version 450 core

out vec4 fragColor;

void main()
{
	fragColor = vec4(0.5, 0.2, 0.8, 1.0);
}