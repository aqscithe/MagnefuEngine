#shader vertex
#version 460 core

const int MAX_POINT_LIGHTS = 3;

struct PointLight 
{
    vec3 LightPos; // Use vec4 to include the padding
    float padding1;

    vec3 LightColor; // Use vec4 to include the padding
    float padding2;

    float MaxLightDist;
    float RadiantFlux;
    int LightEnabled;
    int padding3;
};

// -- Push Constants -- //
//layout(push_constant) uniform PushConstants
//{
//} PC;

// -- Set 0 -- //
layout(set = 0, binding = 0) uniform RenderPassGlobalsUBO
{
    mat4 View;
    mat4 Proj;
    vec3 CameraPos;
    float padding1;

    PointLight PointLights[MAX_POINT_LIGHTS];
    int   LightCount;

} globals_ubo;

// -- Set 1 -- //
layout(set = 1, binding = 0) uniform MaterialUBO
{
    mat4 Model;
    vec3 Tint;
    float Reflectance;
    float Opacity;
} mat_ubo;

// -- In -- //
layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec3 InNormal;
layout(location = 3) in vec3 InTangent;
layout(location = 4) in vec3 InBitangent;
layout(location = 5) in vec2 InTexCoord;

// -- Out -- //
layout(location = 0) out vec2 FragTexCoord;
layout(location = 1) out vec3 TangentCameraPos;
layout(location = 2) out vec3 TangentFragPos;
layout(location = 3) out vec3 TangentLightPos[MAX_POINT_LIGHTS];
//layout(location = 3) out vec3 TangentLightPos;


void main()
{
    gl_Position = globals_ubo.Proj * globals_ubo.View * mat_ubo.Model * vec4(InPosition, 1.0);
    FragTexCoord = InTexCoord;

    // Creating TBN for Normal Map Calculations
    vec3 T = normalize(vec3(mat_ubo.Model * vec4(InTangent, 0.0)));
    vec3 B = normalize(vec3(mat_ubo.Model * vec4(InBitangent, 0.0)));
    vec3 N = normalize(vec3(mat_ubo.Model * vec4(InNormal, 0.0)));
    mat3 TBN = transpose(mat3(T, B, N));

    // Move Light and Camera Pos to Tangent Space
    for (int i = 0; i < globals_ubo.LightCount; i++)
    {
        TangentLightPos[i] = TBN * globals_ubo.PointLights[i].LightPos;
    }
    //TangentLightPos = TBN * globals_ubo.LightPos;
    TangentCameraPos = TBN * globals_ubo.CameraPos;
    TangentFragPos = TBN * vec3(mat_ubo.Model * vec4(InPosition, 1.0));

}


#shader fragment
#version 460 core


const float PI = 3.1415926535897932384626;
const int MAX_POINT_LIGHTS = 3;

struct PointLight
{
    vec3 LightPos; // Use vec4 to include the padding
    float padding1;

    vec3 LightColor; // Use vec4 to include the padding
    float padding2;

    float MaxLightDist;
    float RadiantFlux;
    int LightEnabled;
    int padding3;
};



// -- Push Constants -- //
//layout(push_constant) uniform PushConstants
//{
//    
//} PC;

// -- Set 0 -- //
layout(set = 0, binding = 0) uniform RenderPassGlobalsUBO
{
    mat4 View;
    mat4 Proj;
    vec3 CameraPos;
    float padding1;

    PointLight PointLights[MAX_POINT_LIGHTS];
    int   LightCount;

} globals_ubo;


// TODO: 
    /*
    * 
    * If there are ever performance concerns, Lightenabled could be used strictly cpu side to determine 
    * if light needs to be culled or not.(once i am using an array of lights)
    */


// -- Set 1 -- //
layout(set = 1, binding = 0) uniform MaterialUBO
{
    mat4 Model;
    vec3 Tint;
    float Reflectance;
    float Opacity;
} mat_ubo;

layout(set = 1, binding = 1) uniform sampler2D DiffuseSampler;
layout(set = 1, binding = 2) uniform sampler2D ARMSampler;
layout(set = 1, binding = 3) uniform sampler2D NormalSampler;


// -- In -- //
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec3 TangentCameraPos;
layout(location = 2) in vec3 TangentFragPos;
layout(location = 3) in vec3 TangentLightPos[MAX_POINT_LIGHTS];
//layout(location = 3) in vec3 TangentLightPos;


// -- Out -- //
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
    
    vec3 BRDF = vec3(0.0);
    vec3 BaseColor = vec3(texture(DiffuseSampler, TexCoord));
    vec3 ARM = texture(ARMSampler, TexCoord).rgb;


    float AO = ARM.r;
    float Roughness = ARM.g;
    float Metallic = ARM.b;

    for (int i = 0; i < globals_ubo.LightCount; i++)
    {
        if (globals_ubo.PointLights[i].LightEnabled == 1)
        {

            // Sample Normal Map
            vec3 Normal = texture(NormalSampler, TexCoord).rgb;


            // Simulating a point light
            vec3 LightPosMinusFragPos = TangentLightPos[i] - TangentFragPos;
            vec3 LightVector = normalize(LightPosMinusFragPos);
            vec3 ViewVector = normalize(TangentCameraPos - TangentFragPos);


            // --- Getting Attenuation --- //

            float distance = length(LightPosMinusFragPos);
            float lightIntensity = globals_ubo.PointLights[i].RadiantFlux;

            // Basic Approach
            /*if (distance > PC.MaxLightDist)
            {
                OutColor = vec4(vec3(0.0), PC.Opacity);
                return;
            }*/

            // Smoothstep Function Approach
            float smoothRadius = globals_ubo.PointLights[i].MaxLightDist * 0.8;

            if (distance > smoothRadius)
            {
                float t = (distance - smoothRadius) / (globals_ubo.PointLights[i].MaxLightDist - smoothRadius);
                if (t == 1.0)
                {
                    OutColor = vec4(vec3(0.0), mat_ubo.Opacity);
                    return;
                }
                t = clamp(t, 0.0, 1.0);
                t = t * t * (3.0 - 2.0 * t); // Smoothstep function

                lightIntensity *= 1.0 - t; // Interpolate light intensity towards 0
            }
            //float attenuation = 1.0 / distance
            float attenuation = 1.0 / (distance * distance);
            //float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
            vec3 Radiance = globals_ubo.PointLights[i].LightColor * lightIntensity * attenuation;


            // ---Microfacet BRDF--- //

            // Fresnel Reflectance
            //float Metallic = float(texture(MetalSampler, TexCoord));
            vec3 HalfwayVector = normalize(LightVector + ViewVector);
            vec3 F0 = vec3(0.16 * (mat_ubo.Reflectance * mat_ubo.Reflectance));

            // https://youtu.be/teTroOAGZjM
            // section referring to BSDF lighting
            F0 = mix(F0, BaseColor, Metallic);
            float VoH = clamp(dot(ViewVector, HalfwayVector), 0.0, 1.0);
            vec3 F = FresnelSchlick(F0, VoH);


            // Normal Distribution Function
            //float Roughness = float(texture(RoughnessSampler, TexCoord));
            float D = D_GGX(Roughness, clamp(dot(Normal, HalfwayVector), 0.0, 1.0));

            // Geometry Term
            float NormDotLight = dot(Normal, LightVector);
            float NoL = clamp(NormDotLight, 0.0, 1.0);
            float NoV = clamp(dot(Normal, ViewVector), 0.0, 1.0);
            float G = G_Smith(Roughness, NoL, NoV);

            vec3 spec = (F * D * G) / (4.0 * max(NoL, 0.001) * max(NoV, 0.001));

            vec3 Kd = vec3(1.0) - F;

            Kd *= (1.0 - Metallic);

            // add to outgoing radiance
            float NdotL = max(NormDotLight, 0.0);
            BRDF += (Kd * BaseColor / PI + spec) * Radiance * NdotL;
        }
    }

    vec3 color = BRDF * AO;
    color *= mat_ubo.Tint;

    // Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));


    // Set Final Fragment Color
    //OutColor = vec4(color, mat_ubo.Opacity);
    OutColor = vec4(color, 1.0);

    

}