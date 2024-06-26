#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec4  v_Color;
layout(location = 3) in vec2  v_TexCoords;
layout(location = 4) in float   v_TexID;

uniform mat4 u_MVP;

out vec4 VertexColor;
out vec2 TexCoords;
out float TexID;

void main()
{
	// Must do multiplication in reverse order as I am using row major ordering
	gl_Position = u_MVP * vec4(v_Position, 1.0);
	VertexColor = v_Color;
	TexCoords   = v_TexCoords;
	TexID		= v_TexID;
}


#shader fragment
#version 450 core


uniform sampler2D u_Texture[2];

in  vec4 VertexColor;
in  vec2 TexCoords;
in float TexID;

out vec4 FragColor;

void main()
{
	FragColor = texture(u_Texture[int(ceil(TexID))], TexCoords);
}