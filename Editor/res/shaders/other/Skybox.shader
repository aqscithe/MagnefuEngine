#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;


uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 TexCoords;

void main()
{
	vec4 pos =  u_Projection * u_View * vec4(aPosition, 1.0);
	gl_Position = pos.xyww;
	TexCoords = vec3(aPosition.x, aPosition.y, -aPosition.z);
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