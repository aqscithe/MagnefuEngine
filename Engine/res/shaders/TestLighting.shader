#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec2  v_TexCoords;
layout(location = 3) in float   v_TexID;



uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

out vec3 Normal;
out vec3 FragPos;


void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));
		
}




#shader fragment
#version 450 core

#define NR_POINT_LIGHTS 2
#define NR_SPOT_LIGHTS 1
#define NR_DIR_LIGHTS 1

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

struct DirectionLight
{
	vec3  Direction;
	vec3  Diffuse;
	vec3  Specular;
};

uniform DirectionLight u_DirectionLights[NR_DIR_LIGHTS];

struct SpotLight
{
	vec3  Direction;
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
	float Linear;
	float Quadratic;
	float InnerCutoff;
	float OuterCutoff;
};

uniform SpotLight u_SpotLights[NR_SPOT_LIGHTS];

uniform vec3 u_CameraPos;
uniform int u_ShadingTechnique;
uniform int u_ReflectionModel;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;


float CalculateAttenuation(vec3 position, float constant, float Linear, float quadratic)
{
	float distance = length(position - FragPos);
	return 1.0 / (constant + Linear * distance + quadratic * pow(distance, 2));
}

vec3 GetPhongReflection(vec3 LightVector, vec3 ViewVector, vec3 Specular)
{
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	return  u_material.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * Specular;
}

vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 ViewVector, vec3 Specular)
{
	vec3 HalfwayVector = normalize(LightVector + ViewVector);
	return  u_material.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * Specular;
}

vec3 GetCombined(vec3 DiffuseLight, vec3 SpecularLight)
{
	vec3 Ambient = u_Intensity.Ambient * u_material.Diffuse;
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse;
	vec3 Specular = SpecularLight * u_Intensity.Specular * u_material.Specular;

	return Ambient + Diffuse + Specular;
}


vec3 GetPointReflectionLight(int index)
{
	vec3 LightVector = normalize(u_PointLights[index].Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_PointLights[index].Diffuse;
	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_PointLights[index].Specular) : GetBlinnPhongReflection(LightVector, ViewVector, u_PointLights[index].Specular);

	return GetCombined(DiffuseLight, SpecularLight) * CalculateAttenuation(u_PointLights[index].Position, u_PointLights[index].Constant, u_PointLights[index].Linear, u_PointLights[index].Quadratic);
}

vec3 GetDirReflectionLight(int index)
{
	vec3 LightVector = normalize(-u_DirectionLights[index].Direction);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_DirectionLights[index].Diffuse;
	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_DirectionLights[index].Specular) : GetBlinnPhongReflection(LightVector, ViewVector, u_DirectionLights[index].Specular);

	return GetCombined(DiffuseLight, SpecularLight);
}



vec3 CalculateDirLights()
{
	vec3 result;
	for(int i = 0; i < NR_DIR_LIGHTS; i++)
		result += GetDirReflectionLight(i);
	return result;
}

vec3 CalculatePointLights()
{
	vec3 result;
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		result += GetPointReflectionLight(i);
	return result;
}

vec3 CalculateSpotLights()
{
	vec3 result = vec3(0.0, 0.0, 0.0);

	return result;
}

vec3 ShadeFragment()
{
	vec3 result;
	switch (u_ShadingTechnique)
	{
	case 0:
		result += CalculateDirLights();
		result += CalculatePointLights();
		//result += CalculateSpotLights();
	default:
		break;
	}
	return result;
}

void main()
{
	FragColor = vec4(ShadeFragment(), 1.0);
}