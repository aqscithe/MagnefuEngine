#if defined(VERTEX_CLEAR_DEPTH)

// Full-screen triangle that covers the viewport.
const vec2 positions[3] = vec2[3]( vec2(-1.0, -1.0),
                                    vec2( 3.0, -1.0),
                                    vec2(-1.0,  3.0) );
void main() {
    gl_Position = vec4( positions[gl_VertexIndex], 0.0, 1.0 );
}

#endif // VERTEX_CLEAR_DEPTH

#if defined(GEOMETRY_CLEAR_DEPTH)

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Push constants: base layer to start clearing from, and how many layers to clear.
layout(push_constant) uniform ClearArgs { uint baseLayer; uint layerCount; } args;

void main() {
    for(uint layer = 0u; layer < args.layerCount; ++layer) {
        for(int v = 0; v < 3; ++v) {
            gl_Position = gl_in[v].gl_Position;
            gl_Layer    = int(args.baseLayer + layer);
            EmitVertex();
        }
        EndPrimitive();
    }
}

#endif // GEOMETRY_CLEAR_DEPTH