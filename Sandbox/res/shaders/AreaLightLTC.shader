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

// -- Set 1 -- //
layout(set = 1, binding = 0) uniform MaterialUBO
{
    mat4 Model;
    vec3 Tint;
    float Reflectance;
    float Opacity;
} mat_ubo;



void main()
{
    gl_Position = globals_ubo.Proj * globals_ubo.View * mat_ubo.Model * vec4(InPosition, 1.0);
    FragNormal = InNormal;
    FragTexCoord = InTexCoord;
    FragPosition = vec3(mat_ubo.Model * vec4(InPosition, 1.0));
}

#shader fragment
#version 460 core

// Just create this in sandbox for now
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

layout(set = 0, binding = 1) uniform sampler2D LTC1;
layout(set = 0, binding = 2) uniform sampler2D LTC2;

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


const float LUT_SIZE = 64.0;
const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const float LUT_BIAS = 0.5 / LUT_SIZE;

const float GAMMA = 2.2;

// Vector form without project to the plane (dot with the normal)
// Use for proxy sphere clipping
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

float IntegrateEdge(vec3 v1, vec3 v2)
{
    return IntegrateEdgeVec(v1, v2).z;
}

// P is fragPos in world space (LTC distribution)
vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], int twoSided)
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 4 vertices for clipping)
    vec3 L[4];
    // transform polygon from LTC back to origin Do (cosine weighted)
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light
    vec3 dir = points[0].xyz - P; // LTC space
    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    vec3 vsum = vec3(0.0);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    // form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z / len;
    if (behind)
        z = -z;

    vec2 uv = vec2(z * 0.5f + 0.5f, len); // range [0, 1]
    uv = uv * LUT_SCALE + LUT_BIAS;

    // Fetch the form factor for horizon clipping
    float scale = texture(LTC2, uv).w;

    float sum = len * scale;
    if (!behind && twoSided == 0)
        sum = 0.0;

    // Outgoing radiance (solid angle) for the entire polygon
    vec3 Lo_i = vec3(sum, sum, sum);
    return Lo_i;
}

// PBR-maps for roughness (and metallic) are usually stored in non-linear
// color space (sRGB), so we use these functions to convert into linear RGB.
vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}


vec3 ToLinear(vec3 v) { return PowVec3(v, GAMMA); }
vec3 ToSRGB(vec3 v) { return PowVec3(v, 1.0 / GAMMA); }


void main()
{
    vec3 ARM = texture(ARMSampler, FragTexCoord).rgb;
    vec3 Diffuse = texture(DiffuseSampler, FragTexCoord).xyz;// * vec3(0.7f, 0.8f, 0.96f);

    // gamma correction
    float Metallic = ARM.b;
    vec3 Specular = mix(ToLinear(vec3(0.23f)), Diffuse, Metallic); // Blend based on metallic
    //vec3 Specular = ToLinear(vec3(0.23f, 0.23f, 0.23f)); // Diffuse

    vec3 result = vec3(0.0f);

    // Sample Normal Map
    vec3 Normal = texture(NormalSampler, FragTexCoord).rgb;

    vec3 ViewVector = normalize(globals_ubo.CameraPos - FragPosition);
    float dotNV = clamp(dot(Normal, ViewVector), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    float Roughness = ARM.g;
    vec2 uv = vec2(Roughness, sqrt(1.0f - dotNV));
    uv = uv * LUT_SCALE + LUT_BIAS;

    // get 4 parameters for inverse_M
    vec4 t1 = texture(LTC1, uv);

    // Get 2 parameters for Fresnel calculation
    vec4 t2 = texture(LTC2, uv);

    mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(0, 1, 0),
        vec3(t1.z, 0, t1.w)
    );

    // translate light source for testing
    vec3 translatedPoints[4];
    translatedPoints[0] = PC.AreaLight.Points0.xyz + PC.AreaLight.Translation;
    translatedPoints[1] = PC.AreaLight.Points1.xyz + PC.AreaLight.Translation;
    translatedPoints[2] = PC.AreaLight.Points2.xyz + PC.AreaLight.Translation;
    translatedPoints[3] = PC.AreaLight.Points3.xyz + PC.AreaLight.Translation;

    // Evaluate LTC shading

    vec3 LTC_Diffuse = LTC_Evaluate(Normal, ViewVector, FragPosition, mat3(1), translatedPoints, PC.AreaLight.TwoSided);
    vec3 LTC_Specular = LTC_Evaluate(Normal, ViewVector, FragPosition, Minv, translatedPoints, PC.AreaLight.TwoSided);

    // GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
    LTC_Specular *= Specular * t2.x + (1.0f - Specular) * t2.y;

    result = PC.AreaLight.Color * PC.AreaLight.Intensity * (LTC_Specular + Diffuse * LTC_Diffuse);

    FragColor = vec4(ToSRGB(result), 1.0f);

    //FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}



