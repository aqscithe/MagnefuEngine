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
	vec3        Ka;
	vec3        Kd;
	vec3        Ks;
	float        Ns;
	//float Roughness;
	//float Opacity;
};

uniform Material u_material;

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

uniform float u_RadiantFlux;
uniform float u_PointLightRadius;

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
uniform int u_ReflectionModel;

float PI = 3.1415926535897932384626;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

in vec3 GoraudReflectionResult;

out vec4 FragColor;


//float CalculateAttenuation(vec3 position, float constant, float Linear, float quadratic)
//{
//	float distance = length(position - FragPos);
//	return 1.0 / (constant + Linear * distance + quadratic * pow(distance, 2));
//}
//
//// http://www.cemyuksel.com/research/pointlightattenuation/
//// seems most useful when light source is close to object
//float CalculateNonSingularPointLightAttenuation(vec3 position, float radius)
//{
//	float distance = length(position - FragPos);
//	float squareSum = pow(distance, 2) + pow(radius, 2);
//	return 2.0 / (squareSum + distance * sqrt(squareSum));
//}
//
//vec3 GetPhongReflection(vec3 LightVector, vec3 ViewVector, vec3 Specular)
//{
//	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
//	return  u_material.Ks * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Ns) * Specular;
//}
//
//vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 ViewVector, vec3 Specular)
//{
//	vec3 HalfwayVector = normalize(LightVector + ViewVector);
//	return  u_material.Ks * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Ns) * Specular;
//}
//
//vec3 GetCombined(vec3 DiffuseLight, vec3 SpecularLight)
//{
//	vec3 Ambient = u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
//	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
//	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material.Specular[u_material.TexID], TexCoords));
//
//	return Ambient + Diffuse + Specular;
//}
//
//vec3 GetDirReflectionLight(int index)
//{
//	vec3 LightVector = normalize(-u_DirectionLights[index].Direction);
//	vec3 ViewVector = normalize(u_CameraPos - FragPos);
//
//	vec3 DiffuseLight = u_material.Kd * dot(LightVector, Normal) * u_DirectionLights[index].Diffuse;
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
//		vec3 DiffuseLight = u_material.Kd * dot(LightVector, Normal) * u_SpotLights[index].Diffuse;
//		vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_SpotLights[index].Specular) :
//			GetBlinnPhongReflection(LightVector, ViewVector, u_SpotLights[index].Specular);
//
//		return GetCombined(DiffuseLight * edgeIntensity, SpecularLight * edgeIntensity) *
//			CalculateAttenuation(u_SpotLights[index].Position, u_SpotLights[index].Constant, u_SpotLights[index].Linear, u_SpotLights[index].Quadratic);
//	}
//	else
//	{
//		return u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
//	}
//}
//
//vec3 GetPointReflectionLight(int index)
//{
//	vec3 LightVector = normalize(u_PointLights[index].Position - FragPos);
//	vec3 ViewVector = normalize(u_CameraPos - FragPos);
//
//	vec3 DiffuseLight = u_material.Kd * dot(LightVector, Normal) * u_PointLights[index].Diffuse;
//	vec3 SpecularLight = u_ReflectionModel == 0 ? GetPhongReflection(LightVector, ViewVector, u_PointLights[index].Specular) : GetBlinnPhongReflection(LightVector, ViewVector, u_PointLights[index].Specular);
//
//	float Attenuation = u_PointLightRadius > 0.0 ?
//		CalculateNonSingularPointLightAttenuation(u_PointLights[index].Position, u_PointLightRadius) :
//		CalculateAttenuation(u_PointLights[index].Position, u_PointLights[index].Constant, u_PointLights[index].Linear, u_PointLights[index].Quadratic);
//
//	return GetCombined(DiffuseLight, SpecularLight) * Attenuation;
//}

//satisfies energy conservation unlike classic Phong
vec3 CalcModifiedPhongBRDF(vec3 LightVector, vec3 ViewVector)
{
	vec3 color = (vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Kd) / PI;
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	float Ps = (u_material.Ns + 2.0) / (2.0 * PI); 
	color += pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Ns) * Ps * vec3(texture(u_material.Specular[u_material.TexID], TexCoords)) * u_material.Ks;
	return color; // no color component should be greater than 1.0
}

vec3 CalcPhongBRDF(vec3 LightVector, vec3 ViewVector)
{
	vec3 color = vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Kd;
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	color += pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Ns) * vec3(texture(u_material.Specular[u_material.TexID], TexCoords)) * u_material.Ks;
	return color;
}

vec3 CalcPointLightRadiance(int index)
{
	vec3 Radiance = vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Ka;

	// Calculate Point Irradiance
	vec3 LightVector = normalize(u_PointLights[index].Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	float distance = length(u_PointLights[index].Position - FragPos);

	float Irradiance = u_RadiantFlux * max(dot(LightVector, Normal), 0.0) / (4.0 * PI * distance * distance);

	// Calculate Selected BRDF
	vec3 BRDF = u_ReflectionModel == 0 ? CalcPhongBRDF(LightVector, ViewVector) : CalcModifiedPhongBRDF(LightVector, ViewVector);

	Radiance += BRDF * Irradiance * u_PointLights[index].Color;

	return Radiance;
}


//vec3 CalculateDirLights()
//{
//	vec3 result = vec3(0.0);
//	for (int i = 0; i < NR_DIR_LIGHTS; i++)
//		result += u_DirectionLights[i].Enabled ? GetDirReflectionLight(i) : vec3(0.0);
//	return result;
//}

vec3 CalculatePointLights()
{
	vec3 radiance = vec3(0.0);
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		radiance += u_PointLights[i].Enabled ? CalcPointLightRadiance(i) : vec3(0.0);
	return radiance;
}

//vec3 CalculateSpotLights()
//{
//	vec3 result = vec3(0.0);
//	for (int i = 0; i < NR_SPOT_LIGHTS; i++)
//		result += u_SpotLights[i].Enabled ? GetSpotReflectionLight(i) : vec3(0.0);
//
//	return result;
//}

vec3 ShadeFragment()
{
	vec3 radiance = vec3(0.0);
	switch (u_ShadingTechnique)
	{
	case 0:
		//radiance += CalculateDirLights();
		radiance += CalculatePointLights();
		//radiance += CalculateSpotLights();
		break;
	default:
		break;
	}
	return radiance;
}

void main()
{
	FragColor = vec4(ShadeFragment(), 1.0);
}