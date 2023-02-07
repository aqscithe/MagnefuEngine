#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec4  v_Color;
layout(location = 3) in vec2  v_TexCoords;
layout(location = 4) in float   v_TexID;


struct Light
{
	float K_a;
	float K_d;
	float K_s;
	vec3  Position;
	vec4  Ambient;
	vec4  Diffuse;
	vec4  Specular;
};

uniform Light u_light;
uniform vec3 u_CameraPos;
uniform float u_Shininess; // would be found in a material

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

uniform int u_LightModel;

out vec4 VertexColor;
out vec3 Normal;
out vec3 FragPos;

out vec4 CombinedLight;

vec4 GetCombinedLight(Light u_light, vec3 u_CameraPos, float u_Shininess)
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec4 AmbientLight = u_light.K_a * u_light.Ambient;
	vec4 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;

	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec4 SpecularLight = u_light.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_Shininess) * u_light.Specular;

	return AmbientLight + DiffuseLight + SpecularLight;
}

void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	VertexColor = v_Color;
	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));

	if (u_LightModel == 1)
	{
		CombinedLight = GetCombinedLight(u_light, u_CameraPos, u_Shininess);
	}
}


#shader fragment
#version 450 core


struct Light
{
	float K_a;
	float K_d;
	float K_s;
	vec3  Position;
	vec4  Ambient;
	vec4  Diffuse;
	vec4  Specular;
};

uniform Light u_light;
uniform vec3 u_CameraPos;
uniform float u_Shininess; // would be found in a material
uniform vec3 u_ObjectColor;
uniform int u_LightModel;


in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;

in vec4 CombinedLight;

out vec4 FragColor;



vec4 GetCombinedLight(Light u_light, vec3 u_CameraPos, float u_Shininess)
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec4 AmbientLight = u_light.K_a * u_light.Ambient;
	vec4 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;

	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec4 SpecularLight = u_light.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_Shininess) * u_light.Specular;

	return AmbientLight + DiffuseLight + SpecularLight;
}

void main()
{
	if (u_LightModel == 1)
	{
		FragColor = CombinedLight * vec4(u_ObjectColor, 1.0);
	}
	else
	{
		FragColor = GetCombinedLight(u_light, u_CameraPos, u_Shininess) * vec4(u_ObjectColor, 1.0);
	}
}