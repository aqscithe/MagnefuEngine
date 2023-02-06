#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec4  v_Color;
layout(location = 3) in vec2  v_TexCoords;
layout(location = 4) in float   v_TexID;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;

out vec4 VertexColor;
out vec3 Normal;
out vec3 FragPos;

void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);
	VertexColor = v_Color;
	Normal = v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));
}


#shader fragment
#version 450 core


struct Light
{
	float K_a;
	float K_d;
	vec3  Position;
	vec4  Ambient;
	vec4  Diffuse;
};

uniform Light u_light;
uniform vec3 u_ObjectColor;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

void main()
{
	vec3 diffuseLightVector = normalize(u_light.Position - FragPos);
	vec4 AmbientLight = u_light.K_a * u_light.Ambient;
	vec4 DiffuseLight = u_light.K_d * dot(diffuseLightVector, Normal) * u_light.Diffuse;

	FragColor =  (AmbientLight + DiffuseLight) * vec4(u_ObjectColor, 1.0);
}