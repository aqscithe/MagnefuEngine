#shader vertex
#version 450 core

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 InPosition;
layout(location = 1) in vec3 InColor;
layout(location = 2) in vec2 InTexCoord;

layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec2 FragTexCoord;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(InPosition, 0.0, 1.0);
    FragColor = InColor;
    FragTexCoord = InTexCoord;
}


#shader fragment
#version 450 core

layout(binding = 1) uniform sampler2D TexSampler;

layout(location = 0) in vec3 FragColor;
layout(location = 1) in vec2 FragTexCoord;

layout(location = 0) out vec4 OutColor;

void main()
{
    OutColor = texture(TexSampler, FragTexCoord);
    //OutColor = vec4(FragColor * texture(TexSampler, FragTexCoord).rgb, 1.0);
}