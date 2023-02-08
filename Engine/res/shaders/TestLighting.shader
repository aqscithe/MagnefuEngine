#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec4  v_Color;
layout(location = 3) in vec2  v_TexCoords;
layout(location = 4) in float   v_TexID;

struct Material {
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
	//float Roughness;
	//float Opacity;
};

uniform Material u_material;


struct Light
{
	float K_a;
	float K_d;
	float K_s;
	vec3  Position;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
};

uniform Light u_light;
uniform vec3 u_CameraPos;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

uniform int u_LightModel;

out vec4 VertexColor;
out vec3 Normal;
out vec3 FragPos;

out vec3 CombinedLight;

vec3 GetCombinedLight(Light u_light, vec3 u_CameraPos, Material u_material)
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 AmbientLight = u_light.K_a * u_light.Ambient;
	vec3 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;

	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 SpecularLight = u_light.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_light.Specular;

	return AmbientLight * u_material.Ambient + DiffuseLight * u_material.Diffuse + SpecularLight * u_material.Specular;
}

void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	VertexColor = v_Color;
	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));

	if (u_LightModel == 1)
	{
		CombinedLight = GetCombinedLight(u_light, u_CameraPos, u_material);
	}
}


#shader fragment
#version 450 core



struct Material {
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
	//float Roughness;
	//float Opacity;
};

uniform Material u_material;

struct Light
{
	float K_a;
	float K_d;
	float K_s;
	vec3  Position;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
};

uniform Light u_light;
uniform vec3 u_CameraPos;
uniform int u_LightModel;


in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;

in vec3 CombinedLight;

out vec4 FragColor;



vec3 GetCombinedLight(Light u_light, vec3 u_CameraPos, Material u_material)
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 AmbientLight = u_light.K_a * u_light.Ambient;
	vec3 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;

	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 SpecularLight = u_light.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_light.Specular;

	return AmbientLight * u_material.Ambient + DiffuseLight * u_material.Diffuse + SpecularLight * u_material.Specular;
}

void main()
{
	if (u_LightModel == 1)
	{
		FragColor = vec4(CombinedLight, 1.0);
	}
	else
	{
		FragColor = vec4(GetCombinedLight(u_light, u_CameraPos, u_material), 1.0);
	}
}