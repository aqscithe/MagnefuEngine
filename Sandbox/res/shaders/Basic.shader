#shader vertex
#version 460 core

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec3 InNormal;
layout(location = 3) in vec2 InTexCoord;

layout(location = 0) out vec2 FragTexCoord;
layout(location = 1) out vec3 FragNormal;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(InPosition, 1.0);
    FragTexCoord = InTexCoord;
    FragNormal = InNormal;
}


#shader fragment
#version 460 core

float PI = 3.1415926535897932384626;

layout(push_constant) uniform PushConstants
{
    bool  LightEnabled;
    float Opacity;
    float RadiantFlux;
    float Reflectance; // fresnel reflectance for dielectrics [0.0, 1.0]
    vec3  Tint;
    vec3  CameraPos;
    vec3  LightDirection;
    vec3  LightColor;
    vec3  Ka;
    vec3  Kd;
    vec3  Ks;
} PC;


layout(binding = 1) uniform sampler2D BaseTexSampler;
layout(binding = 2) uniform sampler2D MetalTexSampler;
layout(binding = 3) uniform sampler2D RoughnessTexSampler;

layout(location = 0) in vec2 FragTexCoord;
layout(location = 1) in vec3 FragNormal;

layout(location = 0) out vec4 OutColor;



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
    vec3 Radiance = vec3(texture(BaseTexSampler, FragTexCoord)) * PC.Tint;
    OutColor = vec4(Radiance, 1.0);
}