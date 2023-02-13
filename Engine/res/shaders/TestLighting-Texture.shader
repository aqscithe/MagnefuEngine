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
out vec2 TexCoords;


void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));
	TexCoords = v_TexCoords;
}





#shader fragment
#version 450 core

#define NR_POINT_LIGHTS 2
#define NR_SPOT_LIGHTS 1
#define NR_DIR_LIGHTS 1

struct Material 
{
	unsigned int TexID;
	sampler2D    Diffuse[2];
	sampler2D    Specular[2];
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
in vec2 TexCoords;

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

	vec3 Ambient = u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material.Specular[u_material.TexID], TexCoords));

	return (Ambient + Diffuse + Specular) * CalculateAttenuation(index);

}

vec3 CalculateDirLights()
{
	vec3 result = vec3(0.0, 0.0, 0.0);

	return result;
}

vec3 CalculatePointLights()
{
	vec3 result;
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		result += GetReflectionLight(i);
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