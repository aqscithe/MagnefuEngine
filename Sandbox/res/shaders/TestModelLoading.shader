#shader vertex
#version 460 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec2  v_TexCoords;
layout(location = 3) in float  v_MatID;


uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out float MatID;



void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));
	TexCoords = v_TexCoords;
	MatID = v_MatID;
}





#shader fragment
#version 460 core

// TODO: These values need to be changed programmatically!
#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 1
#define NR_DIR_LIGHTS 1
#define NR_MATS 3

struct Material 
{
	unsigned int MatID;
	sampler2D    Ambient;
	sampler2D    Diffuse;
	sampler2D    Specular;
	vec3         Ka;
	vec3         Kd;
	vec3         Ks;
	float        Ns;
	//sampler2D Roughness;
	//sampler2D Metallic;
	//float Opacity;
};

uniform Material u_material[NR_MATS];

struct Intensity
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};
uniform Intensity u_Intensity;

struct PointLight
{
	bool  Enabled;
	vec3  Position;
	vec3  Color;
	float Constant;
	float Linear;
	float Quadratic;
};

uniform PointLight u_PointLights[NR_POINT_LIGHTS];

struct DirectionLight
{
	bool  Enabled;
	vec3  Direction;
	vec3  Color;
};

uniform DirectionLight u_DirectionLights[NR_DIR_LIGHTS];

struct SpotLight
{
	bool  Enabled;
	vec3  Direction;
	vec3  Position;
	vec3  Color;
	float Constant;
	float Linear;
	float Quadratic;
	float InnerCutoff;
	float OuterCutoff;
};

uniform SpotLight u_SpotLights[NR_SPOT_LIGHTS];

uniform vec3 u_CameraPos;
uniform int u_ShadingTechnique;

// BRDF
// 0 - Phong
// 1 - Blinn-Phong
// 2 - Microfacet (not yet implemented)
uniform int u_ReflectionModel;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in float MatID;

in vec3 GoraudReflectionResult;

out vec4 FragColor;


//float CalculateAttenuation(vec3 position, float constant, float Linear, float quadratic)
//{
//	float distance = length(position - FragPos);
//	return 1.0 / (constant + Linear * distance + quadratic * pow(distance, 2));
//}
//
//vec3 GetPhongReflection(vec3 LightVector, vec3 ViewVector, vec3 Specular)
//{
//	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
//	return  u_material[int(MatID)].Ks * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material[int(MatID)].Ns) * Specular;
//}
//
//vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 ViewVector, vec3 Specular)
//{
//	vec3 HalfwayVector = normalize(LightVector + ViewVector);
//	return  u_material[int(MatID)].Ks * pow(max(dot(Normal, HalfwayVector), 0.0), u_material[int(MatID)].Ns) * Specular;
//}
//
//vec3 GetCombined(vec3 DiffuseLight, vec3 SpecularLight)
//{
//	vec3 Ambient = u_Intensity.Ambient * u_material[int(MatID)].Ka * vec3(texture(u_material[int(MatID)].Ambient, TexCoords));
//	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material[int(MatID)].Diffuse, TexCoords));
//	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material[int(MatID)].Specular, TexCoords));
//
//	return Ambient + Diffuse + Specular;
//}
//
//vec3 GetPointReflectionLight(int index)
//{
//	vec3 LightVector = normalize(u_PointLights[index].Position - FragPos);
//	vec3 ViewVector = normalize(u_CameraPos - FragPos);
//
//	vec3 DiffuseLight = u_material[int(MatID)].Kd * dot(LightVector, Normal) * u_PointLights[index].Diffuse;
//	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_PointLights[index].Specular) : GetBlinnPhongReflection(LightVector, ViewVector, u_PointLights[index].Specular);
//
//	return GetCombined(DiffuseLight, SpecularLight) * 
//		CalculateAttenuation(u_PointLights[index].Position, u_PointLights[index].Constant, u_PointLights[index].Linear, u_PointLights[index].Quadratic);
//}
//
//vec3 GetDirReflectionLight(int index)
//{
//	vec3 LightVector = normalize(-u_DirectionLights[index].Direction);
//	vec3 ViewVector = normalize(u_CameraPos - FragPos);
//
//	vec3 DiffuseLight = u_material[int(MatID)].Kd * dot(LightVector, Normal) * u_DirectionLights[index].Diffuse;
//	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_DirectionLights[index].Specular) :
//		GetBlinnPhongReflection(LightVector, ViewVector, u_DirectionLights[index].Specular);
//
//	return GetCombined(DiffuseLight, SpecularLight);
//}
//
//vec3 GetSpotReflectionLight(int index)
//{
//	vec3 LightVector = normalize(u_SpotLights[index].Position - FragPos);
//
//	float theta = dot(LightVector, normalize(-u_SpotLights[index].Direction));
//	if (theta > u_SpotLights[index].OuterCutoff)
//	{
//		float epsilon = u_SpotLights[index].InnerCutoff - u_SpotLights[index].OuterCutoff;
//		float edgeIntensity = clamp((theta - u_SpotLights[index].OuterCutoff) / epsilon, 0.0, 1.0);
//
//		vec3 ViewVector = normalize(u_CameraPos - FragPos);
//
//		vec3 DiffuseLight = u_material[int(MatID)].Kd * dot(LightVector, Normal) * u_SpotLights[index].Diffuse;
//		vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_SpotLights[index].Specular) :
//			GetBlinnPhongReflection(LightVector, ViewVector, u_SpotLights[index].Specular);
//
//		return GetCombined(DiffuseLight * edgeIntensity, SpecularLight * edgeIntensity) *
//			CalculateAttenuation(u_SpotLights[index].Position, u_SpotLights[index].Constant, u_SpotLights[index].Linear, u_SpotLights[index].Quadratic);
//	}
//	else
//	{
//		return u_Intensity.Ambient * vec3(texture(u_material[int(MatID)].Diffuse, TexCoords));
//	}
//}
//
//vec3 CalculateDirLights()
//{
//	vec3 result = vec3(0.0);
//	for (int i = 0; i < NR_DIR_LIGHTS; i++)
//		result += u_DirectionLights[i].Enabled ? GetDirReflectionLight(i) : vec3(0.0);
//	return result;
//}
//
//vec3 CalculatePointLights()
//{
//	vec3 result = vec3(0.0);
//	for (int i = 0; i < NR_POINT_LIGHTS; i++)
//		result += u_PointLights[i].Enabled ? GetPointReflectionLight(i) : vec3(0.0);
//	return result;
//}
//
//vec3 CalculateSpotLights()
//{
//	vec3 result = vec3(0.0);
//	for (int i = 0; i < NR_SPOT_LIGHTS; i++)
//		result += u_SpotLights[i].Enabled ? GetSpotReflectionLight(i) : vec3(0.0);
//
//	return result;
//}
//
//vec3 ShadeFragment()
//{
//	vec3 result;
//	switch (u_ShadingTechnique)
//	{
//	case 0:
//		result += CalculateDirLights();
//		result += CalculatePointLights();
//		result += CalculateSpotLights();
//		break;
//	default:
//		break;
//	}
//	return result;
//}

void main()
{
	//FragColor = vec4(ShadeFragment(), 1.0);
	FragColor = vec4(vec3(texture(u_material[int(MatID)].Diffuse, TexCoords)), 1.0);
	//FragColor = vec4(1.0);
}