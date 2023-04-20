#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;


uniform mat4 u_MVP;

out vec2 TexCoords;

void main()
{
	gl_Position = u_MVP * vec4(aPosition, 1.0);
	TexCoords = aTexCoords;
}


#shader fragment
#version 450 core

uniform vec3 u_Color;
uniform sampler2D u_DiffuseTexture;

in vec2 TexCoords;

out vec4 fragColor;

void main()
{
	vec3 Diffuse = vec3(texture(u_DiffuseTexture, TexCoords));
	vec3 Blend = mix(Diffuse, u_Color, 0.5);
	fragColor = vec4(Blend, 1.0);
}