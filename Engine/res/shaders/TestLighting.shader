#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec2  v_TexCoords;
layout(location = 3) in float   v_TexID;

#define NR_POINT_LIGHTS 2

struct Material
{
	unsigned int TexID;
	vec3         Diffuse;
	vec3         Specular;
	float        K_d;
	float        K_s;
	float        Shininess;
	//float Roughness;
	//float Opacity;
};

uniform Material u_material;

struct Intensity
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};
uniform Intensity u_Intensity;


struct PointLight
{
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
	float Linear;
	float Quadratic;
};

uniform PointLight u_PointLights[NR_POINT_LIGHTS];

uniform vec3 u_CameraPos;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

uniform int u_ShadingTechnique;
uniform int u_ReflectionModel;

out vec3 Normal;
out vec3 FragPos;

out vec3 GoraudReflectionResult;

float CalculateAttenuation(int index)
{
	float distance = length(u_PointLights[index].Position - FragPos);
	return 1.0 / (u_PointLights[index].Constant + u_PointLights[index].Linear * distance + u_PointLights[index].Quadratic * pow(distance, 2));
}

vec3 GetPhongReflection(vec3 LightVector, vec3 ViewVector, int index)
{
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	return  u_material.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_PointLights[index].Specular;
}

vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 ViewVector, int index)
{
	vec3 HalfwayVector = normalize(LightVector + ViewVector);
	return  u_material.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * u_PointLights[index].Specular;
}

vec3 GetReflectionLight(int index)
{
	vec3 LightVector = normalize(u_PointLights[index].Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_PointLights[index].Diffuse;
	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, index) : GetBlinnPhongReflection(LightVector, ViewVector, index);

	vec3 Ambient = u_Intensity.Ambient * u_material.Diffuse;
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse;
	vec3 Specular = SpecularLight * u_Intensity.Specular * u_material.Specular;

	return (Ambient + Diffuse + Specular) * CalculateAttenuation(index);
}

vec3 CalculatePointLights()
{
	vec3 result;
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		result += GetReflectionLight(i);
	return result;
}

void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));

	vec3 result;

	if (u_ShadingTechnique == 1)
	{
		result += CalculatePointLights();
		GoraudReflectionResult = result;
	}
		
}




#shader fragment
#version 450 core

#define NR_POINT_LIGHTS 2

struct Material
{
	unsigned int TexID;
	vec3         Diffuse;
	vec3         Specular;
	float        K_d;
	float        K_s;
	float        Shininess;
	//float Roughness;
	//float Opacity;
};

uniform Material u_material;

struct Intensity
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};
uniform Intensity u_Intensity;

struct PointLight
{
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
	float Linear;
	float Quadratic;
};

uniform PointLight u_PointLights[NR_POINT_LIGHTS];

uniform vec3 u_CameraPos;
uniform int u_ShadingTechnique;
uniform int u_ReflectionModel;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;

in vec3 GoraudReflectionResult;

out vec4 FragColor;


float CalculateAttenuation(int index)
{
	float distance = length(u_PointLights[index].Position - FragPos);
	return 1.0 / (u_PointLights[index].Constant + u_PointLights[index].Linear * distance + u_PointLights[index].Quadratic * pow(distance, 2));
}


vec3 GetPhongReflection(vec3 LightVector, vec3 ViewVector, int index)
{
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	return  u_material.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_PointLights[index].Specular;
}

vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 ViewVector, int index)
{
	vec3 HalfwayVector = normalize(LightVector + ViewVector);
	return  u_material.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * u_PointLights[index].Specular;
}

vec3 GetReflectionLight(int index)
{
	vec3 LightVector = normalize(u_PointLights[index].Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_PointLights[index].Diffuse;
	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, index) : GetBlinnPhongReflection(LightVector, ViewVector, index);

	vec3 Ambient = u_Intensity.Ambient * u_material.Diffuse;
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse;
	vec3 Specular = SpecularLight * u_Intensity.Specular * u_material.Specular;

	return (Ambient + Diffuse + Specular) * CalculateAttenuation(index);
}

vec3 CalculatePointLights()
{
	vec3 result;
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		result += GetReflectionLight(i);
	return result;
}

void main()
{
	if (u_ShadingTechnique == 1)
		FragColor = vec4(GoraudReflectionResult, 1.0);
	else
	{
		vec3 result = CalculatePointLights();
		FragColor = vec4(result, 1.0);
	}
		
}