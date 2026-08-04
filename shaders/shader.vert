#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : require
#extension GL_EXT_scalar_block_layout : require

//
//const vec3 positions[3] = vec3[]
//(
//        vec3( 0.0, -0.5, 0.0), // Top
//        vec3(-0.5,  0.5, 0.0), // Bottom Left
//        vec3( 0.5,  0.5, 0.0)  // Bottom Right
//);
//
//const vec3 colors[3] = vec3[]
//(
//        vec3(1.0, 0.0, 0.0), // Red
//        vec3(0.0, 1.0, 0.0), // Green
//        vec3(0.0, 0.0, 1.0)  // Blue
//);



layout (location = 0) out vec3 outColor;

// 1. Define the memory layout of a single Vertex
struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

// 2. Map a 64-bit address to an unsized array of Vertex structs
layout(buffer_reference, scalar) readonly buffer VertexBuffer {
    Vertex vertices[];
};

// 3. Define the Push Constants matching your C++ struct
layout(push_constant) uniform PushConstants {
    VertexBuffer vertexBufferAddress; // 64-bit BDA pointer!
//    uint32_t textureIndex;
    vec4 color;
    mat4 modelMatrix;
} pc;

//layout(location = 0) out vec2 outUV;

void main() {
    Vertex v = pc.vertexBufferAddress.vertices[gl_VertexIndex];
    outColor = v.normal;
    // Convert local vertex position to 4D vector (x, y, z, w=1.0)
    vec4 localPosition = vec4(v.pos, 1.0);

    // Transform from Local Space ➔ World Space!
    vec4 worldPosition = pc.modelMatrix * localPosition;

    gl_Position = worldPosition;



//    debugPrintfEXT("Processing Vertex Index: %d\n", gl_VertexIndex);
//    // 4. Dereference VRAM memory directly using gl_VertexIndex!
//    Vertex v = pc.vertexBufferAddress.vertices[gl_VertexIndex];
//    debugPrintfEXT("Processing Vertex Index2: %d\n", gl_VertexIndex);
//    gl_Position = vec4(v.position, 1.0);
//    gl_Position = pc.modelMatrix * vec4(v.position, 1.0);
//    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
//    outUV = v.uv;
}


//#version 460
//
//const vec3 positions[3] = vec3[]
//(
//        vec3( 0.0, -0.5, 0.0), // Top
//        vec3(-0.5,  0.5, 0.0), // Bottom Left
//        vec3( 0.5,  0.5, 0.0)  // Bottom Right
//);
//
//const vec3 colors[3] = vec3[]
//(
//        vec3(1.0, 0.0, 0.0), // Red
//        vec3(0.0, 1.0, 0.0), // Green
//        vec3(0.0, 0.0, 1.0)  // Blue
//);
//
//
//void main() {
//    // gl_VertexIndex represent the current vertex index we're processing
//    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
//    outColor = colors[gl_VertexIndex];
//}
