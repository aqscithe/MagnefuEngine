#shader vertex
#version 460 core



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

// -- Set 0 -- //
layout(set = 0, binding = 0) uniform RenderPassUBO
{
    mat4 View;
    mat4 Proj;
    vec3 CameraPos;
} globals_ubo;


void main()
{
    gl_Position = globals_ubo.Proj * globals_ubo.View * mat_ubo.Model * vec4(InPosition, 1.0);
    FragNormal = InNormal
        FragTexCoord = InTexCoord;
    FragPosition = vec3(mat_ubo.Model * vec4(InPosition, 1.0));
}

// END OF VERTEX SHADER

#shader fragment
#version 460 core


struct AreaLight {
    vec4  Points0;
    vec4  Points1;
    vec4  Points2;
    vec4  Points3;
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

// -- Out -- //
layout(location = 0) out vec4 FragColor;



// -- Push Constants -- //
layout(push_constant) uniform PushConstants
{
    AreaLight AreaLight;
} PC;

// -- Set 0 -- //
layout(set = 0, binding = 0) uniform RenderPassUBO
{
    mat4 View;
    mat4 Proj;
    vec3 CameraPos;
} globals_ubo;


// -- Set 1 -- //
layout(set = 1, binding = 0) uniform MaterialUBO
{
    mat4 Model;
    vec3 Tint;
    float Reflectance;
    float Opacity;
} mat_ubo;

//layout(set = 1, binding = 1) uniform sampler2D DiffuseSampler;
//layout(set = 1, binding = 2) uniform sampler2D ARMSampler;
//layout(set = 1, binding = 3) uniform sampler2D NormalSampler;

void main()
{
    vec3 color = PC.AreaLight.Color;
    color *= mat_ubo.Tint;

    // Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    // Set Final Fragment Color
    //OutColor = vec4(color, mat_ubo.Opacity);
    OutColor = vec4(color, 1.0);

}

// END OF FRAGMENT SHADER



