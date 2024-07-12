#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;


uniform mat4 u_MVP; 
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
	gl_Position = u_MVP * vec4(aPosition, 1.0);
	TexCoords = aTexCoords;
	Normal = mat3(u_NormalMatrix) * aNormal;
	//Normal = aNormal;
	FragPos = vec3(u_ModelMatrix * vec4(aPosition, 1.0));
}


#shader fragment
#version 450 core

float PI = 3.1415926535897932384626;

uniform vec3 u_CameraPos;

uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform bool u_LightEnabled;

uniform float u_Opacity;
uniform float u_RadiantFlux;
uniform float u_Reflectance = 0.5; // fresnel reflectance for dielectrics [0.0, 1.0]
uniform vec3 u_Ka = vec3(0.0);
uniform vec3 u_Kd = vec3(1.0);
uniform vec3 u_Ks = vec3(1.0);

uniform sampler2D u_DiffuseTexture; // diffuse for dielectrics, F0 for metals
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_MetallicTexture;

uniform vec3 u_Tint = vec3(1.0);

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

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

void main()
{
	vec3 Radiance;
	vec3 BaseColor = vec3(texture(u_DiffuseTexture, TexCoords));
	if (u_LightEnabled)
	{
		// AMBIENT PORTION          
		Radiance = BaseColor * u_Ka;

		vec3 LightVector = normalize(-u_LightDirection);
		vec3 ViewVector = normalize(u_CameraPos - FragPos);
		float Irradiance = u_RadiantFlux;

		// ---Microfacet BRDF--- //

		// Fresnel Reflectance
		float metallic = float(texture(u_MetallicTexture, TexCoords));
		vec3 HalfwayVector = normalize(LightVector + ViewVector);
		vec3 F0 = vec3(0.16 * (u_Reflectance * u_Reflectance));

		// https://youtu.be/teTroOAGZjM
		// section referring to BSDF lighting
		F0 = mix(F0, BaseColor, metallic);  // a and b may need to be flipped
		float VoH = clamp(dot(ViewVector, HalfwayVector), 0.0, 1.0);
		vec3 F = FresnelSchlick(F0, VoH);

		// Normal Distribution Function
		float roughness = float(texture(u_RoughnessTexture, TexCoords));
		float D = D_GGX(roughness, clamp(dot(Normal, HalfwayVector), 0.0, 1.0));

		// Geometry Term
		float NoL = clamp(dot(Normal, LightVector), 0.0, 1.0);
		float NoV = clamp(dot(Normal, ViewVector), 0.0, 1.0);
		float G = G_Smith(roughness, NoL, NoV);

		vec3 spec = F * D * G / 4.0 * max(NoL, 0.001) * max(NoV, 0.001) * u_Ks;

		vec3 rhod = BaseColor * u_Kd;
		rhod *= vec3(1.0) - F;

		rhod *= (1.0 - metallic);

		vec3 diff = rhod / PI;

		vec3 BRDF = diff + spec;

		Radiance += BRDF * Irradiance * u_LightColor;
		Radiance *= u_Tint;
	}
	else
	{
		Radiance = BaseColor * u_Tint;
	}

	FragColor = vec4(Radiance, u_Opacity);
}