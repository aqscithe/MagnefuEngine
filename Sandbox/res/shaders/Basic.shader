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
layout(location = 2) out vec3 FragPos;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(InPosition, 1.0);
    FragTexCoord = InTexCoord;
    FragNormal = InNormal;
    FragPos = vec3(ubo.model * vec4(InPosition, 1.0));
}


#shader fragment
#version 460 core

float PI = 3.1415926535897932384626;

layout(push_constant) uniform PushConstants
{
    vec3  Tint;
    vec3  CameraPos;
    vec3  LightPos;
    vec3  LightColor;
    vec3  Ka;
    vec3  Kd;
    vec3  Ks;
    float Opacity;
    float RadiantFlux;
    float Reflectance; // fresnel reflectance for dielectrics [0.0, 1.0]
    int   LightEnabled;
} PC;


layout(binding = 1) uniform sampler2D BaseTexSampler;
layout(binding = 2) uniform sampler2D MetalTexSampler;
layout(binding = 3) uniform sampler2D RoughnessTexSampler;
layout(binding = 4) uniform sampler2D NormalTexSampler;

layout(location = 0) in vec2 FragTexCoord;
layout(location = 1) in vec3 FragNormal;
layout(location = 2) in vec3 FragPos;  // World Position

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

// TODO: Add attenuation - Is radiant flux attenuation?


void main()
{
    vec3 Radiance;
    vec3 BaseColor = vec3(texture(BaseTexSampler, FragTexCoord));
    if (PC.LightEnabled == 1)
    {
        // AMBIENT PORTION          
        Radiance = BaseColor * PC.Ka;

        // Simulating a point light
        vec3 LightVector = normalize(PC.LightPos - FragPos);
        vec3 ViewVector = normalize(PC.CameraPos - FragPos);
        float Irradiance = PC.RadiantFlux;

        // Getting Attenuation
        /*float distance = length(PC.LightPos - FragPos);
        float attenuation = 1.0 / (distance * distance);*/

        // ---Microfacet BRDF--- //

        // Fresnel Reflectance
        float metallic = float(texture(MetalTexSampler, FragTexCoord));
        vec3 HalfwayVector = normalize(LightVector + ViewVector);
        vec3 F0 = vec3(0.16 * (PC.Reflectance * PC.Reflectance));

        // https://youtu.be/teTroOAGZjM
        // section referring to BSDF lighting
        F0 = mix(F0, BaseColor, metallic);  // a and b may need to be flipped
        float VoH = clamp(dot(ViewVector, HalfwayVector), 0.0, 1.0);
        vec3 F = FresnelSchlick(F0, VoH);

        // Normal Distribution Function
        float roughness = float(texture(RoughnessTexSampler, FragTexCoord));
        float D = D_GGX(roughness, clamp(dot(FragNormal, HalfwayVector), 0.0, 1.0));

        // Geometry Term
        float NoL = clamp(dot(FragNormal, LightVector), 0.0, 1.0);
        float NoV = clamp(dot(FragNormal, ViewVector), 0.0, 1.0);
        float G = G_Smith(roughness, NoL, NoV);

        vec3 spec = F * D * G / 4.0 * max(NoL, 0.001) * max(NoV, 0.001) * PC.Ks;

        vec3 rhod = BaseColor * PC.Kd;
        rhod *= vec3(1.0) - F;

        rhod *= (1.0 - metallic);

        vec3 diff = rhod / PI;

        vec3 BRDF = diff + spec;

        //Radiance += BRDF * Irradiance * PC.LightColor * attenuation;
        Radiance += BRDF * Irradiance * PC.LightColor;
        Radiance *= PC.Tint;
    }
    else
    {
        Radiance = BaseColor * PC.Tint;
    }

    OutColor = vec4(Radiance, PC.Opacity);

}