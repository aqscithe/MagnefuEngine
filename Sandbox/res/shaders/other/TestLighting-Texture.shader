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
	sampler2D	 Roughness[2];
	sampler2D    Metallic[2];
	vec3        Ka;
	vec3        Kd;
	vec3        Ks;
	float        Ns;
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

uniform float u_Reflectance;

float PI = 3.1415926535897932384626;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;


out vec4 FragColor;


float CalculateAttenuation(vec3 position, float constant, float Linear, float quadratic)
{
	float distance = length(position - FragPos);
	return 1.0 / (constant + Linear * distance + quadratic * pow(distance, 2));
}

// http://www.cemyuksel.com/research/pointlightattenuation/
// seems most useful when light source is close to object
float CalculateNonSingularPointLightAttenuation(vec3 position, float radius)
{
	float distance = length(position - FragPos);
	float squareSum = pow(distance, 2) + pow(radius, 2);
	return 2.0 / (squareSum + distance * sqrt(squareSum));
}


vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float roughness, float NoH)
{
	float r_squared = roughness * roughness;
	return r_squared / (PI * pow((NoH * NoH) * (r_squared - 1.0) + 1.0, 2));
}

float G_Schlick_GGX(float roughness, float cosTheta)
{
	float k = roughness / 2.0;
	return max(cosTheta, 0.001) / (cosTheta * (1.0 - k) + k);
}

float G_Smith(float roughness, float NoL, float NoV)
{
	return G_Schlick_GGX(roughness, NoL) * G_Schlick_GGX(roughness, NoV);
}

float F_Schlick90(float cosTheta, float F0, float F90)
{
	return F0 + (F90 - F0) * pow(1.0 - cosTheta, 5.0);
}

float disneyDiffFactor(float roughness, float NoV, float NoL, float VoH)
{
	float r_squared = roughness * roughness;
	float F90 = 0.5 + 2.0 * r_squared * VoH * VoH;
	float FIn = F_Schlick90(NoL, 1.0, F90);
	float FOut = F_Schlick90(NoV, 1.0, F90);
	return FIn * FOut;
}

// Cook-Torrance
vec3 CalcMicroFacetBRDF(vec3 LightVector, vec3 ViewVector)
{
	// Fresnel Reflectance
	float metallic = float(texture(u_material.Metallic[u_material.TexID], TexCoords));
	vec3 HalfwayVector = normalize(LightVector + ViewVector);
	vec3 F0 = vec3(0.16 * (u_Reflectance * u_Reflectance));

	// https://youtu.be/teTroOAGZjM
	// section referring to BSDF lighting
	F0 = mix(F0, vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)), metallic);
	float VoH = clamp(dot(ViewVector, HalfwayVector), 0.0, 1.0);
	vec3 F = FresnelSchlick(F0, VoH );

	// Normal Distribution Function
	float roughness = float(texture(u_material.Roughness[u_material.TexID], TexCoords));
	float D = D_GGX(roughness, clamp(dot(Normal, HalfwayVector), 0.0, 1.0));

	// Geometry Term
	float NoL = clamp(dot(Normal, LightVector), 0.0, 1.0);
	float NoV = clamp(dot(Normal, ViewVector), 0.0, 1.0);
	float G = G_Smith(roughness, NoL, NoV);

	// not sure if I should be multiplying by the specular texture here
	//vec3 spec = F * D * G / 4.0 * max(NoL, 0.001) * max(NoV, 0.001);
	vec3 spec = (F * D * G / 4.0 * max(NoL, 0.001) * max(NoV, 0.001)) * vec3(texture(u_material.Specular[u_material.TexID], TexCoords)) * u_material.Ks;

	vec3 rhod = vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Kd;
	rhod *= vec3(1.0) - F;

	//optional disney diffuse factor(more expensive)
	//rhod *= disneyDiffFactor(roughness, NoV, NoL, VoH);

	rhod *= (1.0 - metallic);

	vec3 diff = rhod / PI;

	return diff + spec;
}

vec3 CalcBlinnPhongBRDF(vec3 LightVector, vec3 ViewVector)
{
	vec3 color = vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Kd;
	vec3 HalfwayVector = normalize(LightVector + ViewVector);
	color += pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Ns) * vec3(texture(u_material.Specular[u_material.TexID], TexCoords)) * u_material.Ks;
	return  color;
}

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

vec3 GetBRDF(vec3 LightVector, vec3 ViewVector)
{
	switch (u_ReflectionModel)
	{
	case 0:
		return CalcPhongBRDF(LightVector, ViewVector);
	case 1:
		return CalcModifiedPhongBRDF(LightVector, ViewVector);
		break;
	case 2:
		return CalcBlinnPhongBRDF(LightVector, ViewVector);
		break;
	case 3:
		return CalcMicroFacetBRDF(LightVector, ViewVector);
	default:
		return CalcBlinnPhongBRDF(LightVector, ViewVector);
	}
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
	vec3 BRDF = GetBRDF(LightVector, ViewVector);
	
	Radiance += BRDF * Irradiance * u_PointLights[index].Color;

	float Attenuation = u_PointLightRadius > 0.0 ?
				CalculateNonSingularPointLightAttenuation(u_PointLights[index].Position, u_PointLightRadius) :
				CalculateAttenuation(u_PointLights[index].Position, u_PointLights[index].Constant, u_PointLights[index].Linear, u_PointLights[index].Quadratic);

	return Radiance * Attenuation;
}

vec3 CalcSpotLightRadiance(int index)
{
	vec3 Radiance = vec3(0.0);
	vec3 LightVector = normalize(u_SpotLights[index].Position - FragPos);

	float theta = dot(LightVector, normalize(-u_SpotLights[index].Direction));
	if (theta > u_SpotLights[index].OuterCutoff)
	{
		Radiance = vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Ka;

		float epsilon = u_SpotLights[index].InnerCutoff - u_SpotLights[index].OuterCutoff;
		float edgeIntensity = clamp((theta - u_SpotLights[index].OuterCutoff) / epsilon, 0.0, 1.0);

		vec3 ViewVector = normalize(u_CameraPos - FragPos);


		float distance = length(u_SpotLights[index].Position - FragPos);

		float Irradiance = u_RadiantFlux * max(dot(LightVector, Normal), 0.0) / (4.0 * PI * distance * distance);

		vec3 BRDF = GetBRDF(LightVector, ViewVector);

		Radiance += BRDF * edgeIntensity * Irradiance * u_SpotLights[index].Color;

		float Attenuation = CalculateAttenuation(u_SpotLights[index].Position, u_SpotLights[index].Constant, u_SpotLights[index].Linear, u_SpotLights[index].Quadratic);

		return Radiance * Attenuation;
	}
	
	return Radiance;
}

vec3 CalcDirLightRadiance(int index)
{
	vec3 Radiance = vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords)) * u_material.Ka;

	vec3 LightVector = normalize(-u_DirectionLights[index].Direction);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	float Irradiance = u_RadiantFlux;

	vec3 BRDF = GetBRDF(LightVector, ViewVector);

	Radiance += BRDF * Irradiance * u_DirectionLights[index].Color;

	return Radiance;
}


vec3 CalculateDirLights()
{
	vec3 result = vec3(0.0);
	for (int i = 0; i < NR_DIR_LIGHTS; i++)
		result += u_DirectionLights[i].Enabled ? CalcDirLightRadiance(i) : vec3(0.0);
	return result;
}

vec3 CalculatePointLights()
{
	vec3 radiance = vec3(0.0);
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		radiance += u_PointLights[i].Enabled ? CalcPointLightRadiance(i) : vec3(0.0);
	return radiance;
}

vec3 CalculateSpotLights()
{
	vec3 result = vec3(0.0);
	for (int i = 0; i < NR_SPOT_LIGHTS; i++)
		result += u_SpotLights[i].Enabled ? CalcSpotLightRadiance(i) : vec3(0.0);

	return result;
}

vec3 ShadeFragment()
{
	vec3 radiance = vec3(0.0);
	switch (u_ShadingTechnique)
	{
	case 0:
		radiance += CalculateDirLights();
		radiance += CalculatePointLights();
		radiance += CalculateSpotLights();
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