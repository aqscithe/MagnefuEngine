#shader vertex
#version 450 core

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
    );

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
    );

layout(location = 0) out vec3 color;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    color = colors[gl_VertexIndex];
}


#shader fragment
#version 450 core

layout(location = 0) in vec3 color;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}