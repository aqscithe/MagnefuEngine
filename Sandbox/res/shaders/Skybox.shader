#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;


uniform mat4 u_View;

out vec3 TexCoords;

void main()
{
	TexCoords = aPosition;
	vec4 pos = vec4(aPosition, 1.0) * u_View;
	gl_Position = pos.xyww;
}


#shader fragment
#version 450 core

uniform samplerCube u_DiffuseTexture;

in vec3 TexCoords;

out vec4 fragColor;

void main()
{
	fragColor = texture(u_DiffuseTexture, TexCoords);
}