#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec2  v_TexCoords;
layout(location = 3) in float   v_TexID;

struct Material {
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
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


struct Light
{
	float K_d;
	float K_s;
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
};

uniform Light u_light;
uniform vec3 u_CameraPos;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

uniform int u_ShadingTechnique;
uniform int u_ReflectionModel;

out vec3 Normal;
out vec3 FragPos;

out vec3 ReflectionResult;

vec3 GetPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_light.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_light.Specular;

	return u_Intensity.Ambient * u_material.Diffuse + DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse + SpecularLight * u_Intensity.Specular * u_material.Specular;
}

vec3 GetBlinnPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 HalfwayVector = normalize(LightVector + ViewVector);

	vec3 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_light.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * u_light.Specular;

	return u_Intensity.Ambient * u_material.Diffuse + DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse + SpecularLight * u_Intensity.Specular * u_material.Specular;
}

void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));

	if (u_ShadingTechnique == 1)
	{
		if (u_ReflectionModel == 1)
		{
			ReflectionResult = GetBlinnPhongReflection();
		}
		else
		{
			ReflectionResult = GetPhongReflection();
		}
		
	}
}


#shader fragment
#version 450 core



struct Material {
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
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

struct Light
{
	float K_d;
	float K_s;
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
};

uniform Light u_light;
uniform vec3 u_CameraPos;
uniform int u_ShadingTechnique;
uniform int u_ReflectionModel;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;

in vec3 ReflectionResult;

out vec4 FragColor;



vec3 GetPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;

	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 SpecularLight = u_light.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_light.Specular;

	return u_Intensity.Ambient * u_material.Diffuse + DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse + SpecularLight * u_Intensity.Specular * u_material.Specular;
}

vec3 GetBlinnPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 HalfwayVector = normalize(LightVector + ViewVector);

	vec3 DiffuseLight = u_light.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_light.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * u_light.Specular;

	return u_Intensity.Ambient * u_material.Diffuse + DiffuseLight * u_Intensity.Diffuse * u_material.Diffuse + SpecularLight * u_Intensity.Specular * u_material.Specular;
}

void main()
{
	if (u_ShadingTechnique == 1)
	{
		FragColor = vec4(ReflectionResult, 1.0);
	}
	else
	{
		if (u_ReflectionModel == 1)
		{
			FragColor = vec4(GetBlinnPhongReflection(), 1.0);
		}
		else
		{
			FragColor = vec4(GetPhongReflection(), 1.0);
		}
		
	}
}