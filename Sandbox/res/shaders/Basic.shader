#shader vertex
#version 450 core

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
    );

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}

#shader fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(0.5, 0.2, 0.8, 1.0);
}