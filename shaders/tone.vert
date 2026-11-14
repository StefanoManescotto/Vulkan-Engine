#version 460

layout(location = 0) out vec2 vUV;

void main() {
    // Fullscreen triangle from gl_VertexIndex: (0,0), (2,0), (0,2)
    vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(vUV * 2.0 - 1.0, 0.0, 1.0);
}
