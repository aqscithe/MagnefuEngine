#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTexCoords;


uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;

out vec3 FragPos;
out vec3 VertexColor;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
	gl_Position = u_MVP * vec4(aPosition, 1.0);
	VertexColor = aColor;
	TexCoords = aTexCoords;
	Normal = aNormal;
	FragPos = vec3(u_ModelMatrix * vec4(aPosition, 1.0));
}


#shader fragment
#version 450 core

float PI = 3.1415926535897932384626;

uniform vec3 u_CameraPos;

uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform bool u_LightEnabled;

uniform float u_RadiantFlux;

uniform sampler2D u_DiffuseTexture;

in vec3 FragPos;
in vec3 VertexColor;
in vec3 Normal;
in vec2 TexCoords;

out vec4 fragColor;

void main()
{					// Default
	vec3 radiance = VertexColor;
	if (u_LightEnabled)
	{
					// ambient color * ka
		radiance = VertexColor * vec3(0.0);

		vec3 LightVector = normalize(-u_LightDirection);
		vec3 ViewVector = normalize(u_CameraPos - FragPos);
		float Irradiance = u_RadiantFlux;

		// Blinn-Phong BRDF //

					// diffuse color  * kd / pi
		vec3 BRDF = (VertexColor * vec3(1.0)) / PI;
		vec3 ReflectionVector = 2 * dot(LightVector, Normal) * Normal - LightVector;

					// shininess + ...
		float Ps = (32.0 + 2.0) / (2.0 * PI);
																  // shininess(32)      // specular color * ks
		BRDF += pow(max(dot(ReflectionVector, ViewVector), 0.0), 32.0) * Ps * (VertexColor * vec3(0.75));

		radiance += BRDF * Irradiance * u_LightColor;
	}

	fragColor = vec4(radiance, 1.0);
}