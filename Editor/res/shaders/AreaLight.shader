#shader vertex
#version 460 core

const int MAX_LIGHTS = 3;

struct AreaLight {
    vec3  Color;
    float padding1;
    vec3  Translation;
    float padding2;
    float Intensity;
    int   TwoSided;
};

// -- In -- //
layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec3 InNormal;
layout(location = 3) in vec3 InTangent;
layout(location = 4) in vec3 InBitangent;
layout(location = 5) in vec2 InTexCoord;

// -- Out -- //
layout(location = 0) out vec3 FragPosition;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec2 FragTexCoord;
layout(location = 3) out flat int InstanceIndex;


// --Push Constants -- //
layout(push_constant) uniform PushConstants
{
    float Roughness;

} PC;


// -- Set 0 -- //
layout(set = 0, binding = 0) uniform RenderPassUBO
{
    mat4 View;
    mat4 Proj;
    vec3 CameraPos;

    AreaLight AreaLight[MAX_LIGHTS];
    mat4 AreaLightPoints;
    int AreaLightCount;
} globals_ubo;

// -- Set 1 -- //
layout(set = 1, binding = 0) uniform MaterialUBO
{
    mat4 Model[MAX_LIGHTS];
} mat_ubo;


void main()
{
    InstanceIndex = gl_InstanceIndex;
    gl_Position = globals_ubo.Proj * globals_ubo.View * mat_ubo.Model[InstanceIndex] * vec4(InPosition, 1.0);
    FragNormal = InNormal;
    FragTexCoord = InTexCoord;
    FragPosition = vec3(mat_ubo.Model[InstanceIndex] * vec4(InPosition, 1.0));
    
}

// END OF VERTEX SHADER

#shader fragment
#version 460 core

const int MAX_LIGHTS = 3;


struct AreaLight {
    vec3  Color;
    float padding1;
    vec3  Translation;
    float padding2;
    float Intensity;
    int   TwoSided;
};


// -- In -- //
layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragNormal;
layout(location = 2) in vec2 FragTexCoord;
layout(location = 3) in flat int InstanceIndex;


// -- Out -- //
layout(location = 0) out vec4 FragColor;



// -- Push Constants -- //
layout(push_constant) uniform PushConstants
{
    float Roughness;

} PC;

// -- Set 0 -- //
layout(set = 0, binding = 0) uniform RenderPassUBO
{
    mat4 View;
    mat4 Proj;
    vec3 CameraPos;

    AreaLight AreaLight[MAX_LIGHTS];
    mat4 AreaLightPoints;
    int AreaLightCount;
} globals_ubo;

layout(set = 0, binding = 1) uniform sampler2D LTC1;
layout(set = 0, binding = 2) uniform sampler2D LTC2;


// -- Set 1 -- //
layout(set = 1, binding = 0) uniform MaterialUBO
{
    mat4 Model[MAX_LIGHTS];
} mat_ubo;

//layout(set = 1, binding = 1) uniform sampler2D DiffuseSampler;
//layout(set = 1, binding = 2) uniform sampler2D ARMSampler;
//layout(set = 1, binding = 3) uniform sampler2D NormalSampler;

void main()
{
    // Each instance needs to know which color it should be
    //vec3 color = { 1.0, 1.0, 1.0 };
    vec3 color = globals_ubo.AreaLight[InstanceIndex].Color;

    // Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    // Set Final Fragment Color
    //OutColor = vec4(color, mat_ubo.Opacity);
    FragColor = vec4(color, 1.0);

}

// END OF FRAGMENT SHADER



