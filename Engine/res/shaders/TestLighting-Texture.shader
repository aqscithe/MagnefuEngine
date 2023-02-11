#shader vertex
#version 450 core

layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec3  v_Normal;
layout(location = 2) in vec2  v_TexCoords;
layout(location = 3) in float   v_TexID;

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


struct Light
{
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
	float Linear;
	float Quadratic;
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
out vec2 TexCoords;

out vec3 ReflectionResult;

vec3 GetPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_material.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_light.Specular;

	vec3 Ambient = u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material.Specular[u_material.TexID], TexCoords));

	return Ambient + Diffuse + Specular;
}

vec3 GetBlinnPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 HalfwayVector = normalize(LightVector + ViewVector);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_material.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * u_light.Specular;

	vec3 Ambient = u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material.Specular[u_material.TexID], TexCoords));

	return Ambient + Diffuse + Specular;
}

void main()
{
	gl_Position = u_MVP * vec4(v_Position, 1.0);

	Normal = mat3(u_NormalMatrix) * v_Normal;
	FragPos = vec3(u_ModelMatrix * vec4(v_Position, 1.0));
	TexCoords = v_TexCoords;

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

struct Light
{
	vec3  Position;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
	float Linear;
	float Quadratic;
};

uniform Light u_light;
uniform vec3 u_CameraPos;
uniform int u_ShadingTechnique;
uniform int u_ReflectionModel;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

in vec3 ReflectionResult;

out vec4 FragColor;



vec3 GetPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;
	vec3 ViewVector = normalize(u_CameraPos - FragPos);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_material.K_s * pow(max(dot(ReflectionVector, ViewVector), 0.0), u_material.Shininess) * u_light.Specular;

	vec3 Ambient = u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material.Specular[u_material.TexID], TexCoords));

	return Ambient + Diffuse + Specular;
}

vec3 GetBlinnPhongReflection()
{
	vec3 LightVector = normalize(u_light.Position - FragPos);
	vec3 ViewVector = normalize(u_CameraPos - FragPos);
	vec3 HalfwayVector = normalize(LightVector + ViewVector);

	vec3 DiffuseLight = u_material.K_d * dot(LightVector, Normal) * u_light.Diffuse;
	vec3 SpecularLight = u_material.K_s * pow(max(dot(Normal, HalfwayVector), 0.0), u_material.Shininess) * u_light.Specular;

	vec3 Ambient = u_Intensity.Ambient * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Diffuse = DiffuseLight * u_Intensity.Diffuse * vec3(texture(u_material.Diffuse[u_material.TexID], TexCoords));
	vec3 Specular = SpecularLight * u_Intensity.Specular * vec3(texture(u_material.Specular[u_material.TexID], TexCoords));

	return Ambient + Diffuse + Specular;
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