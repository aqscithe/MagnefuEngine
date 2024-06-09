#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec2  v_TexCoords;
layout(location = 3) in float   v_TexID;


uniform mat4 u_MVP;


void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);
}


#shader fragment
#version 450 core



out vec4 FragColor;



void main()
{
	FragColor = vec4(1.0);
}