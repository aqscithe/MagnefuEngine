#shader vertex
#version 450 core

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

void main() {

    gl_PointSize = 4.0;
    gl_Position = vec4(inPosition.xy, 0.1, 1.0);
    fragColor = inColor.rgb;
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {

    vec2 coord = gl_PointCoord - vec2(0.5);
    outColor = vec4(fragColor, 0.5 - length(coord));
}

#shader compute
#version 450 core

struct Particle {
    vec2 position;
    vec2 velocity;
    vec4 color;
};

layout(binding = 0) uniform ParameterUBO
{
    float deltaTime;
} ubo;

layout(std140, binding = 1) readonly buffer ParticleSSBOIn 
{
    Particle particlesIn[];
};

layout(std140, binding = 2) buffer ParticleSSBOOut 
{
    Particle particlesOut[];
};

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main()
{
    uint index = gl_GlobalInvocationID.x;

    Particle particleIn = particlesIn[index];

    particlesOut[index].position = particleIn.position + particleIn.velocity.xy * ubo.deltaTime;
    particlesOut[index].velocity = particleIn.velocity;

    // Flip movement at window border
    if ((particlesOut[index].position.x <= -1.0) || (particlesOut[index].position.x >= 1.0)) {
        particlesOut[index].velocity.x = -particlesOut[index].velocity.x;
    }
    if ((particlesOut[index].position.y <= -1.0) || (particlesOut[index].position.y >= 1.0)) {
        particlesOut[index].velocity.y = -particlesOut[index].velocity.y;
    }

}